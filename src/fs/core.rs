use anyhow::{bail, Context, Result};
use fuser::{
    FileAttr, FileType, Filesystem, MountOption, ReplyAttr, ReplyData, ReplyDirectory, ReplyEntry,
    Request,
};
use libc::ENOENT;
use reqwest::{
    self,
    header::{self, HeaderMap},
};
use serde::{Deserialize, Serialize};
use std::{ffi::OsStr, time::SystemTime};

use std::time::{Duration, UNIX_EPOCH};

use crate::qbt::core::Client;
use crate::qbt::torrents::Item;
use crate::qbt::torrents::Torrent;
use log::{debug, error, info, warn};

const TTL: Duration = Duration::from_secs(1);
const BLOCK_SIZE: u32 = 512;
const INODE_START: usize = 128;

#[derive(Debug, Clone, Copy)]
enum InodeKind {
    File,
    Directory,
}

#[derive(Debug, Clone)]
pub struct Inode<'a> {
    number: u64,
    kind: InodeKind,

    pub name: String,
    pub torrent: Option<&'a Torrent<'a>>,
    pub item: Option<Item<'a>>,
}

impl From<InodeKind> for FileType {
    fn from(item: InodeKind) -> Self {
        match item {
            InodeKind::File => Self::RegularFile,
            InodeKind::Directory => Self::Directory,
        }
    }
}

const ROOT_INODE_NUMBER: u64 = 1;
const INODE_OFFSET: usize = 100_000;

pub struct QFS<'a> {
    client: &'a Client,
    torrents: Vec<Torrent<'a>>,
}

impl<'a> QFS<'a> {
    pub fn reload(&mut self) -> Result<()> {
        self.client.get_torrent_list(&mut self.torrents);
        Ok(())
    }

    /// Get the inode of a node by path.
    pub fn find(&self, path: &str) -> Option<&Inode> {
        let path_components: Vec<&str> = path.split("/").collect();
        let mut cwd = ROOT_INODE_NUMBER;

        // The first path component will always be part of a torrent.
        // Subsequent path members are "contents" of a torrent.

        if path_components.len() == 1 {
            // Search the torrents for the matching name.
            let name = path_components.last();

            for t in &self.torrents {
                let content_path = &t.info.content_path;
                println!("FIND: {:?} -> {:?}", path, content_path);
            }
        }
        None
    }

    pub fn new(client: &'a Client) -> Result<Self> {
        let root_inode = Inode {
            number: ROOT_INODE_NUMBER,
            kind: InodeKind::Directory,
            name: "/".to_string(),
            torrent: None,
            item: None,
        };
        let mut me = Self {
            client,
            torrents: vec![],
        };

        let torrents: Vec<Torrent<'a>> = vec![];

        Ok(me)
    }
}

impl<'a> Filesystem for QFS<'a> {
    fn lookup(&mut self, _req: &Request, parent: u64, name: &OsStr, reply: ReplyEntry) {
        println!("LOOKUP\n  PARENT: {:?}\n  NAME: {:?}", parent, name);

        if parent == ROOT_INODE_NUMBER {
            for (i, t) in self.torrents.iter().enumerate() {
                if *name != *t.info.name {
                    continue;
                }

                let ino = (i + 1) * INODE_OFFSET;
                let timestamp = t.fetch_time;
                let my_attr = FileAttr {
                    ino: ino as u64,
                    size: 0,
                    blocks: 0,
                    atime: timestamp,
                    mtime: timestamp,
                    ctime: timestamp,
                    crtime: timestamp,
                    kind: FileType::Directory,
                    perm: 0o755,
                    nlink: 1,
                    uid: 1000,
                    gid: 1000,
                    rdev: 0,
                    flags: 0,
                    blksize: BLOCK_SIZE,
                };
                reply.entry(&TTL, &my_attr, 0);
                return;
            }
            reply.error(ENOENT);
            return;
        }

        let now = SystemTime::now();
        let mut my_attr = FileAttr {
            ino: 0,
            size: 0,
            blocks: 0,
            atime: now,
            mtime: now,
            ctime: now,
            crtime: now,
            kind: FileType::RegularFile,
            perm: 0o600,
            nlink: 1,
            uid: 1000,
            gid: 1000,
            rdev: 0,
            flags: 0,
            blksize: BLOCK_SIZE,
        };

        let clean = name.to_str();
        match clean {
            Some("metadata") => {
                let ino = ((parent + 1) as usize * INODE_OFFSET) + 1;
                my_attr.ino = ino as u64;
                reply.entry(&TTL, &my_attr, 0);
            }
            Some("control") => {
                let ino = ((parent + 1) as usize * INODE_OFFSET) + 2;
                my_attr.ino = ino as u64;
                reply.entry(&TTL, &my_attr, 0);
            }
            Some(&_) => reply.error(ENOENT),
            None => {
                panic!("Invalid filename: {:?}", name)
            }
        }

        return;
    }

    fn getattr(&mut self, _req: &Request, ino: u64, _fh: Option<u64>, reply: ReplyAttr) {
        println!("GETATTR\n  INO: {:?}\n", ino);
        if ino == ROOT_INODE_NUMBER {
            const ROOT_ATTR: FileAttr = FileAttr {
                ino: ROOT_INODE_NUMBER,
                size: 0,
                blocks: 0,
                atime: UNIX_EPOCH,
                mtime: UNIX_EPOCH,
                ctime: UNIX_EPOCH,
                crtime: UNIX_EPOCH,
                kind: FileType::Directory,
                perm: 0o755,
                nlink: 2,
                uid: 1000,
                gid: 1000,
                rdev: 0,
                flags: 0,
                blksize: BLOCK_SIZE,
            };

            reply.attr(&TTL, &ROOT_ATTR);
            return;
        }

        let my_attr = FileAttr {
            ino,
            size: 0,
            blocks: 0,
            atime: UNIX_EPOCH,
            mtime: UNIX_EPOCH,
            ctime: UNIX_EPOCH,
            crtime: UNIX_EPOCH,
            kind: FileType::RegularFile,
            perm: 0o644,
            nlink: 2,
            uid: 1000,
            gid: 1000,
            rdev: 0,
            flags: 0,
            blksize: BLOCK_SIZE,
        };

        reply.attr(&TTL, &my_attr);
        return;
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
        todo!();
    }

    fn readdir(
        &mut self,
        _req: &Request,
        ino: u64,
        _fh: u64,
        offset: i64,
        mut reply: ReplyDirectory,
    ) {
        let base_entries = vec![
            (2, FileType::Directory, "."),
            (3, FileType::Directory, ".."),
        ];

        if offset == 0 {
            for (i, t) in base_entries.iter().enumerate() {
                reply.add(t.0, (i + 1) as i64, t.1, t.2);
            }
        }

        if ino == ROOT_INODE_NUMBER {
            // Each torrent is represented as a subdirectory, regardless of how
            // the actual data is layed out within the torrent. This is a
            // necessary abstraction, since QBT doesn't expose a cheap way to
            // determine if a torrent is a single file or many directories.

            for (i, t) in self.torrents.iter().enumerate().skip(offset as usize) {
                // Assume the inode for a torrent is it's index * 100_000.
                // This limits each torrent to have 100_000 items, but greatly
                // simplifies the unique inode handling logic, as we can just
                // increment past the inode for all contents.  This does have
                // the implication that a single torrent cannot contain more
                // than 100_000 items within it.

                // Reserve the first INODE_OFFSET inodes
                let idx = i + 1;

                let my_ino = idx * INODE_OFFSET;
                // println!("  INODE: {:?}", ino);

                let full = reply.add(
                    my_ino as u64,
                    idx as i64,
                    FileType::Directory,
                    t.info.name.clone(),
                );
                let full = false;
                if full {
                    println!("  BROKE: {:?}", self.torrents.len());
                    break;
                }
            }

            reply.ok();
            return;
        }

        // Calculate the position within the torrents vector.
        let torrents_index = (ino as usize) / INODE_OFFSET;
        if torrents_index > self.torrents.len() {
            // This should never happen, we're trying to query an inode that
            // doesn't exist.
            reply.error(ENOENT);
            return;
        }

        // All torrents contain a few magic files.
        const ENTRIES: &[&str] = &["metadata", "control"];
        let torrent = &self.torrents[torrents_index];

        for (i, f) in ENTRIES.iter().enumerate().skip(offset as usize) {
            let ino = ino + 1;
            let idx = torrents_index + 1;
            let full = reply.add(ino as u64, (i + 1) as i64, FileType::RegularFile, f);
            if (full) {
                let name = &torrent.info.name;
                warn!("Full on returning magic files for torrent {name}");
                break;
            }
        }
        reply.ok();
    }
}
