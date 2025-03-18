use anyhow::{bail, Context, Result};
use fuser::{
    FileAttr, FileType, Filesystem, MountOption, ReplyAttr, ReplyData, ReplyDirectory, ReplyEntry,
    ReplyOpen, Request,
};

use fuser::consts::FOPEN_DIRECT_IO;
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

use std::result;

use strum::IntoEnumIterator;
use strum_macros::EnumIter;

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

#[derive(Debug, EnumIter, Copy, Clone, PartialEq, Eq)]
enum FileEntry {
    Metadata = 1,
    ContentInfo,
    Control,
}

impl Into<&'static str> for FileEntry {
    fn into(self) -> &'static str {
        match self {
            Self::Metadata => "metadata",
            Self::ContentInfo => "content_info",
            Self::Control => "control",
        }
    }
}

impl TryFrom<&str> for FileEntry {
    type Error = ();

    fn try_from(value: &str) -> Result<Self, Self::Error> {
        match value {
            "metadata" => Ok(Self::Metadata),
            "content_info" => Ok(Self::ContentInfo),
            "control" => Ok(Self::Control),
            _ => Err(()),
        }
    }
}

impl<'a> QFS<'a> {
    pub fn reload(&mut self) -> Result<()> {
        self.client.get_torrent_list(&mut self.torrents);
        Ok(())
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
            size: 10,
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

        let Some(name_str) = name.to_str() else {
            reply.error(ENOENT);
            return;
        };

        let clean = match FileEntry::try_from(name_str) {
            Ok(x) => x,
            Err(_) => {
                reply.error(ENOENT);
                return;
            }
        };

        let ino = ((parent + 1) as usize * INODE_OFFSET) + (clean as usize);
        my_attr.ino = ino as u64;
        reply.entry(&TTL, &my_attr, 0);
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
        let buffer = "Foobarbaz\n".as_bytes();
        reply.data(&buffer);
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

        let torrent = &self.torrents[torrents_index];

        for (i, f) in FileEntry::iter().enumerate().skip(offset as usize) {
            let fname: &str = f.into();
            let ino = ino + 1;
            let idx = torrents_index + 1;
            let full = reply.add(ino as u64, (i + 1) as i64, FileType::RegularFile, fname);
            if (full) {
                let name = &torrent.info.name;
                warn!("Full on returning magic files for torrent {name}");
                break;
            }
        }
        reply.ok();
    }
}
