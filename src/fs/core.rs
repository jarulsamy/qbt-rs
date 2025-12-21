use anyhow::{bail, Context, Error, Result};
use fuser::{
    FileAttr, FileType, Filesystem, MountOption, ReplyAttr, ReplyData, ReplyDirectory, ReplyEntry,
    ReplyOpen, Request,
};

use fuser::consts::FOPEN_DIRECT_IO;
use indextree::{Arena, NodeId};
use libc::ENOENT;
use reqwest::{
    self,
    header::{self, HeaderMap},
};
use serde::{Deserialize, Serialize};
use std::collections::HashMap;
use std::{ffi::OsStr, time::SystemTime};

use std::time::{Duration, UNIX_EPOCH};

use crate::fs::QfsError;
use crate::qbt::core::Client;
use crate::qbt::torrents::Item;
use crate::qbt::torrents::Torrent;
use log::{debug, error, info, warn};

use std::result;

use camino::{Utf8Path, Utf8PathBuf};
use strum::{FromRepr, IntoEnumIterator};
use strum_macros::EnumIter;

const TTL: Duration = Duration::from_secs(1);
const BLOCK_SIZE: u32 = 512;
const INODE_START: usize = 128;

pub type Inode = u64;

const ROOT_INODE_NUMBER: Inode = 1;

#[derive(Debug, Default)]
pub struct InodeAllocator {
    next: Inode,
}

impl InodeAllocator {
    pub fn alloc(&mut self) -> Inode {
        let ino = self.next;
        self.next += 1;
        ino
    }
}

#[derive(Debug)]
pub enum NodeKind {
    Directory { entries: HashMap<String, NodeId> },
    File { size: u64 },
}

#[derive(Debug)]
pub struct Node {
    pub inode: Inode,
    pub kind: NodeKind,
}

#[derive(Debug)]
pub struct Qfs<'a> {
    pub inodes: InodeAllocator,
    pub inode_map: HashMap<Inode, NodeId>,
    pub client: &'a Client,
    pub arena: Arena<Node>,
    pub root: NodeId,
    // torrents: Vec<Torrent<'a>>,
}

fn filetype(node: &Node) -> fuser::FileType {
    return match node.kind {
        NodeKind::Directory { .. } => FileType::Directory,
        NodeKind::File { .. } => FileType::RegularFile,
    };
}

fn default_file_attr(node: &Node) -> FileAttr {
    let kind = filetype(node);
    let size = match node.kind {
        NodeKind::File { size } => size,
        _ => 0,
    };

    FileAttr {
        ino: node.inode,
        size,
        blocks: 1,
        atime: SystemTime::now(),
        mtime: SystemTime::now(),
        ctime: SystemTime::now(),
        crtime: SystemTime::now(),
        kind,
        perm: match kind {
            FileType::Directory => 0o755,
            FileType::RegularFile => 0o644,
            FileType::Symlink => 0o777,
            _ => 0o644,
        },
        nlink: match kind {
            FileType::Directory => 2, // minimum for "." + ".."
            _ => 1,
        },
        uid: 1000,
        gid: 1000,
        rdev: 0,
        flags: 0,
        blksize: 512,
    }
}

impl<'a> Qfs<'a> {
    pub fn new(client: &'a Client) -> Result<Self> {
        let mut arena = Arena::new();

        let root = arena.new_node(Node {
            kind: NodeKind::Directory {
                entries: HashMap::new(),
            },
            inode: ROOT_INODE_NUMBER,
        });
        let mut inode_map = HashMap::new();
        inode_map.insert(ROOT_INODE_NUMBER, root);

        let mut me = Self {
            inodes: InodeAllocator {
                next: ROOT_INODE_NUMBER + 1,
            },
            inode_map: inode_map,
            client: client,
            arena: arena,
            root: root,
        };

        Ok(me)
    }

    pub fn reload(&mut self) -> Result<(), Error> {
        self.arena = Arena::new();

        let root = self.arena.new_node(Node {
            kind: NodeKind::Directory {
                entries: HashMap::new(),
            },
            inode: ROOT_INODE_NUMBER,
        });
        let mut inode_map = HashMap::new();
        inode_map.insert(ROOT_INODE_NUMBER, root);

        self.inodes = InodeAllocator {
            next: ROOT_INODE_NUMBER + 1,
        };

        let mut torrent_list = vec![];
        self.client.get_torrent_list(&mut torrent_list)?;
        for torrent in torrent_list.iter() {
            let name = torrent.info.name.clone();
            let path = Utf8PathBuf::from(name);
            self.mkdir(path);
        }

        Ok(())
    }

    fn resolve(&self, path: &Utf8Path) -> Option<NodeId> {
        let mut cur = self.root;
        if path.as_str() == "/" {
            return Some(cur);
        }

        for comp in path.components() {
            let name = comp.as_str();
            let node = self.arena.get(cur)?;
            match &node.get().kind {
                NodeKind::Directory { entries } => {
                    cur = *entries.get(name)?;
                }
                _ => return None,
            }
        }

        Some(cur)
    }

    fn mkdir(&mut self, path: Utf8PathBuf) -> Result<NodeId, QfsError> {
        let parent_path = path.parent().ok_or(QfsError::InvalidPath)?;
        let name = path.file_name().ok_or(QfsError::InvalidPath)?;
        let parent = self.resolve(parent_path).ok_or(QfsError::NotFound)?;
        match &self.arena[parent].get().kind {
            NodeKind::Directory { .. } => {}
            _ => return Err(QfsError::NotDirectory),
        }

        if self.resolve(&path).is_some() {
            return Err(QfsError::AlreadyExists);
        }

        let inode = self.inodes.alloc();
        let node = self.arena.new_node(Node {
            inode: inode,
            kind: NodeKind::Directory {
                entries: HashMap::new(),
            },
        });

        parent.append(node, &mut self.arena);

        if let NodeKind::Directory { entries } = &mut self.arena[parent].get_mut().kind {
            entries.insert(name.to_string(), node);
        }

        Ok(node)
    }
}

impl<'a> Filesystem for Qfs<'a> {
    fn lookup(&mut self, _req: &Request, parent: u64, name: &OsStr, reply: ReplyEntry) {
        info!("LOOKUP: {parent:?}, {name:?}");
        let ttl = Duration::new(1, 0);

        let parent_id = match self.inode_map.get(&parent) {
            Some(&nid) => nid,
            None => {
                reply.error(ENOENT);
                return;
            }
        };

        let parent_node = &self.arena[parent_id].get();

        let entries = match &parent_node.kind {
            NodeKind::Directory { entries } => entries,
            _ => {
                reply.error(ENOENT);
                return;
            }
        };

        let name_str = match name.to_str() {
            Some(s) => s,
            None => {
                reply.error(ENOENT);
                return;
            }
        };

        let child_id = match entries.get(name_str) {
            Some(&nid) => nid,
            None => {
                reply.error(ENOENT);
                return;
            }
        };

        let child = &self.arena[child_id].get();

        reply.entry(&ttl, &default_file_attr(child), 0);
    }

    fn getattr(&mut self, _req: &Request, ino: u64, _fh: Option<u64>, reply: ReplyAttr) {
        info!("GETATTR: {ino:?}");
        let ttl = Duration::new(1, 0);

        let node_id = match self.inode_map.get(&ino) {
            Some(&nid) => nid,
            None => {
                reply.error(ENOENT);
                return;
            }
        };

        let node = &self.arena[node_id].get();

        reply.attr(&ttl, &default_file_attr(node));
    }

    fn read(
        &mut self,
        _req: &Request,
        ino: u64,
        fh: u64,
        offset: i64,
        size: u32,
        flags: i32,
        lock: Option<u64>,
        reply: ReplyData,
    ) {
        info!("READ: {ino:?}");
    }

    fn readdir(
        &mut self,
        _req: &Request,
        ino: u64,
        _fh: u64,
        offset: i64,
        mut reply: ReplyDirectory,
    ) {
        info!("READDIR: {ino:?}, {offset:?}");

        let Some(&dir_id) = self.inode_map.get(&ino) else {
            reply.error(ENOENT);
            return;
        };

        let dir_node = &self.arena[dir_id];
        let NodeKind::Directory { entries } = &dir_node.get().kind else {
            reply.error(ENOENT);
            return;
        };

        // FUSE offset protocol:
        // offset == 0 → start
        // offset > 0 → resume after that index
        let mut index: i64 = 0;

        if offset <= index {
            if reply.add(ino, index + 1, FileType::Directory, ".") {
                reply.ok();
                return;
            }
        }
        index += 1;

        let parent_ino = dir_node
            .parent()
            .map(|p| self.arena[p].get().inode)
            .unwrap_or(ino);

        if offset <= index {
            if reply.add(parent_ino, index + 1, FileType::Directory, "..") {
                reply.ok();
                return;
            }
        }
        index += 1;

        // 3️⃣ Directory entries
        for (name, &child_id) in entries {
            let child = &self.arena[child_id].get();
            let child_ino = child.inode;
            let kind = filetype(&child);

            if offset <= index {
                if reply.add(child_ino, index + 1, kind, name) {
                    reply.ok();
                    return;
                }
            }
            index += 1;
        }

        reply.ok();
    }
}
