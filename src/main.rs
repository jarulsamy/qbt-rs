use qbt_rs::fs::core::Qfs;
// mod qbt;
use anyhow::{bail, Context, Result};
use qbt_rs::fs;
use qbt_rs::qbt;
use qbt_rs::shell::shell;

use env_logger::Builder;
use fuser::{
    FileAttr, FileType, Filesystem, MountOption, ReplyAttr, ReplyData, ReplyDirectory, ReplyEntry,
    Request,
};
use libc::ENOENT;
use log::LevelFilter;
use std::ffi::OsStr;
use std::time::{Duration, UNIX_EPOCH};

fn qbt_test() {
    let base_url = "REDACTED";
    let username = "REDACTED";
    let password = "REDACTED";

    let mut qbt = qbt::core::Client::new(base_url, username, password, false).unwrap();
    qbt.login().unwrap();

    println!("Version: {}", qbt.get_version().unwrap());
    println!("API Version: {}", qbt.get_api_version().unwrap());
    println!("Build Info: {:?}", qbt.get_build_info().unwrap());
    println!(
        "Default save path: {}",
        qbt.get_default_save_path().unwrap()
    );
    println!(
        "Global transfer info: {:?}",
        qbt.get_global_transfer_info().unwrap()
    );

    println!(
        "Alternative Speed Limits Enabled: {}",
        qbt.alternative_speed_limits_enabled().unwrap()
    );

    println!(
        "Global Download Limit: {}",
        qbt.get_global_download_limit().unwrap()
    );
    println!(
        "Global Upload Limit: {}",
        qbt.get_global_upload_limit().unwrap()
    );

    let mut torrents: Vec<qbt::torrents::Torrent> = vec![];
    qbt.get_torrent_list(&mut torrents).unwrap();
    println!("Torrent List!");
    for i in torrents {
        println!("============================");
        println!("{:?}", i);
        println!("{:?}", i.get_generic_properties().unwrap());

        let mut items = vec![];
        i.get_contents(&mut items).unwrap();
        for i in items {
            println!("{:?}", i);
        }

        // return;
    }

    qbt.logout().unwrap();
}

fn fuse_test() -> Result<()> {
    let mountpoint = "/home/joshua/git/qbt-rs/mount";

    let base_url = "REDACTED";
    let username = "REDACTED";
    let password = "REDACTED";

    let mut qbt = qbt::core::Client::new(base_url, username, password, false).unwrap();

    let options = vec![MountOption::RO, MountOption::FSName("Qfs".to_string())];
    let mut fs = Qfs::new(&qbt)?;
    fs.reload()?;
    fuser::mount2(fs, mountpoint, &options).unwrap();

    Ok(())
}

fn fs_test() {

    // let fs = Filesystem::new(&qbt).unwrap();
    // fs.print_cwd();
    // println!("{:?}", fs);
}

fn main() {
    // Setup logging
    Builder::new().filter_level(LevelFilter::Info).init();

    // qbt_test();
    // fs_test();

    // shell().unwrap();

    fuse_test().unwrap();
}
