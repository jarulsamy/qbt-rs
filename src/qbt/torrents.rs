use anyhow::Result;
use chrono::DateTime;
use chrono::{Local, TimeZone};
use serde::{Deserialize, Serialize};
use serde_repr::{Deserialize_repr, Serialize_repr};
use std::{str::Split, time::SystemTime};

use crate::qbt::core::Client;

#[derive(Serialize, Deserialize, Debug, Clone)]
pub enum TorrentState {
    #[serde(rename = "error")]
    Error,
    #[serde(rename = "missingFiles")]
    MissingFiles,
    #[serde(rename = "uploading")]
    Uploading,
    #[serde(rename = "pausedUP")]
    PausedUp,
    #[serde(rename = "queuedUP")]
    QueuedUp,
    #[serde(rename = "stalledUP")]
    StalledUp,
    #[serde(rename = "stoppedUP")]
    StoppedUp,
    #[serde(rename = "checkingUP")]
    CheckingUp,
    #[serde(rename = "forcedUp")]
    ForcedUp,
    #[serde(rename = "allocating")]
    Allocating,
    #[serde(rename = "downloading")]
    Downloading,
    #[serde(rename = "metaDL")]
    MetaDl,
    #[serde(rename = "pausedDL")]
    PausedDl,
    #[serde(rename = "queuedDL")]
    QueuedDl,
    #[serde(rename = "stalledDL")]
    StalledDl,
    #[serde(rename = "checkingDL")]
    CheckingDl,
    #[serde(rename = "forcedDL")]
    ForcedDl,
    #[serde(rename = "stoppedDL")]
    StoppedDl,
    #[serde(rename = "checkingResumeData")]
    CheckingResumeData,
    #[serde(rename = "moving")]
    Moving,
    #[serde(rename = "unknown")]
    Unknown,
}

#[derive(Clone, Serialize, Deserialize, Debug)]
pub struct TorrentInfo {
    /// Time (Unix Epoch) when the torrent was added to the client
    pub added_on: i64,
    /// Amount of data left to download (bytes)
    pub amount_left: i64,
    /// Whether this torrent is managed by Automatic Torrent Management
    pub auto_tmm: bool,
    /// Percentage of file pieces currently available
    pub availability: f32,
    /// Category of the torrent
    pub category: String,
    /// Amount of transfer data completed (bytes)
    pub completed: i64,
    /// Time (Unix Epoch) when the torrent completed
    pub completion_on: i64,
    /// Absolute path of torrent content (root path for multifile torrents,
    /// absolute file path for singlefile torrents)
    pub content_path: String,
    /// Torrent download speed limit (bytes/s). -1 if unlimited.
    pub dl_limit: i64,
    /// Torrent download speed (bytes/s)
    pub dlspeed: i64,
    /// Amount of data downloaded
    pub downloaded: i64,
    /// Amount of data downloaded this session
    pub downloaded_session: i64,
    /// Torrent ETA (seconds)
    pub eta: i64,
    /// True if first last piece are prioritized
    pub f_l_piece_prio: bool,
    /// True if force start is enabled for this torrent
    pub force_start: bool,
    /// Torrent hash
    pub hash: String,
    /// Last time (Unix Epoch) when a chunk was downloaded/uploaded
    pub last_activity: i64,
    /// Magnet URI corresponding to this torrent
    pub magnet_uri: String,
    /// Maximum share ratio until torrent is stopped from seeding/uploading
    pub max_ratio: f32,
    /// Maximum seeding time (seconds) until torrent is stopped from seeding
    pub max_seeding_time: i64,
    /// Torrent name
    pub name: String,
    /// Number of seeds in the swarm
    pub num_complete: i64,
    /// Number of leechers in the swarm
    pub num_incomplete: i64,
    /// Number of leechers connected to
    pub num_leechs: i64,
    /// Number of seeds connected to
    pub num_seeds: i64,
    /// Torrent priority. Returns -1 if queuing is disabled or torrent is in
    /// seed mode
    pub priority: i32,
    /// Torrent progress (percentage/100)
    pub progress: f32,
    /// Torrent share ratio. Max ratio value: 9999.
    pub ratio: f64,
    /// TODO (what is different from max_ratio?)
    pub ratio_limit: f32,
    /// Path where this torrent's data is stored
    pub save_path: String,
    /// Torrent elapsed time while complete (seconds)
    pub seeding_time: i64,
    /// TODO (what is different from max_seeding_time?) seeding_time_limit is a
    /// per torrent setting, when Automatic Torrent Management is disabled,
    /// furthermore then max_seeding_time is set to seeding_time_limit for this
    /// torrent. If Automatic Torrent Management is enabled, the value is -2.
    /// And if max_seeding_time is unset it have a default value -1.
    pub seeding_time_limit: i64,
    /// Time (Unix Epoch) when this torrent was last seen complete
    pub seen_complete: i64,
    /// True if sequential download is enabled
    pub seq_dl: bool,
    /// Total size (bytes) of files selected for download
    pub size: i64,
    /// Torrent state. See table here below for the possible values
    pub state: TorrentState,
    /// True if super seeding is enabled
    pub super_seeding: bool,
    /// Comma-concatenated tag list of the torrent
    pub tags: String,
    /// Total active time (seconds)
    pub time_active: i64,
    /// Total size (bytes) of all file in this torrent (including unselected
    /// ones)
    pub total_size: i64,
    /// The first tracker with working status. Returns empty string if no
    /// tracker is working.
    pub tracker: String,
    /// Torrent upload speed limit (bytes/s). -1 if unlimited.
    pub up_limit: i64,
    /// Amount of data uploaded
    pub uploaded: i64,
    /// Amount of data uploaded this session
    pub uploaded_session: i64,
    /// Torrent upload speed (bytes/s)
    #[serde(rename = "upspeed")]
    pub up_speed: i64,
}

#[derive(Serialize, Deserialize, Debug)]
pub struct GenericInfo {
    /// Torrent save path
    pub save_path: String,
    /// Torrent creation date (Unix timestamp)
    pub creation_date: i64,
    /// Torrent piece size (bytes)
    pub piece_size: i64,
    /// Torrent comment
    pub comment: String,
    /// Total data wasted for torrent (bytes)
    pub total_wasted: i64,
    /// Total data uploaded for torrent (bytes)
    pub total_uploaded: i64,
    /// Total data uploaded this session (bytes)
    pub total_uploaded_session: i64,
    /// Total data downloaded for torrent (bytes)
    pub total_downloaded: i64,
    /// Total data downloaded this session (bytes)
    pub total_downloaded_session: i64,
    /// Torrent upload limit (bytes/s)
    pub up_limit: i64,
    /// Torrent download limit (bytes/s)
    pub dl_limit: i64,
    /// Torrent elapsed time (seconds)
    pub time_elapsed: i64,
    /// Torrent elapsed time while complete (seconds)
    pub seeding_time: i64,
    /// Torrent connection count
    pub nb_connections: i64,
    /// Torrent connection count limit
    pub nb_connections_limit: i64,
    /// Torrent share ratio
    pub share_ratio: f32,
    /// When this torrent was added (unix timestamp)
    pub addition_date: i64,
    /// Torrent completion date (unix timestamp)
    pub completion_date: i64,
    /// Torrent creator
    pub created_by: String,
    /// Torrent average download speed (bytes/second)
    pub dl_speed_avg: i64,
    /// Torrent download speed (bytes/second)
    pub dl_speed: i64,
    /// Torrent ETA (seconds)
    pub eta: i64,
    /// Last seen complete date (unix timestamp)
    pub last_seen: i64,
    /// Number of peers connected to
    pub peers: i64,
    /// Number of peers in the swarm
    pub peers_total: i64,
    /// Number of pieces owned
    pub pieces_have: i64,
    /// Number of pieces of the torrent
    pub pieces_num: i64,
    /// Number of seconds until the next announce
    pub reannounce: i64,
    /// Number of seeds connected to
    pub seeds: i64,
    /// Number of seeds in the swarm
    pub seeds_total: i64,
    /// Torrent total size (bytes)
    pub total_size: i64,
    /// Torrent average upload speed (bytes/second)
    pub up_speed_avg: i64,
    /// Torrent upload speed (bytes/second)
    pub up_speed: i64,
}

#[derive(Clone, Serialize_repr, Deserialize_repr, Debug)]
#[repr(u8)]
pub enum ItemPriority {
    DoNotDownload = 0,
    Normal = 1,
    High = 6,
    Maximal = 7,
}

#[derive(Clone, Serialize, Deserialize, Debug)]
pub struct Item<'a> {
    /// File index
    pub index: i64,
    /// File name (including relative path)
    pub name: String,
    /// File size (bytes)
    pub size: i64,
    /// File progress (percentage/100)
    pub progress: f32,
    /// File priority
    pub priority: ItemPriority,
    /// True if file is seeding/complete
    pub is_seed: Option<bool>,
    /// (starting piece index, ending piece index) (inclusive)
    pub piece_range: (i64, i64),
    /// Percentage of file pieces currently available (percentage/100)
    pub availability: f32,

    #[serde(skip)]
    pub torrent: Option<&'a Torrent<'a>>,
}

#[derive(Clone, Debug)]
pub struct Torrent<'a> {
    pub client: &'a Client,
    pub info: TorrentInfo,
    pub fetch_time: SystemTime,

    /// Lazy computable raw buffer of metadata.
    metadata_buffer: Option<Vec<u8>>,
}

impl<'a> Client {
    pub fn get_torrent_list<C>(&'a self, container: &mut C) -> Result<()>
    where
        C: Extend<Torrent<'a>>,
    {
        let endpoint = self.url("torrents/info");
        let resp = self.session.get(endpoint).send()?;
        let torrent_infos: Vec<TorrentInfo> = resp.json::<Vec<TorrentInfo>>()?;

        container.extend(torrent_infos.iter().map(|x| Torrent::new(&self, x.clone())));

        Ok(())
    }
}

impl<'a> Torrent<'a> {
    pub fn new(client: &'a Client, info: TorrentInfo) -> Self {
        Self {
            client,
            info,
            fetch_time: SystemTime::now(),
            metadata_buffer: None,
        }
    }

    /* Following are all part of the core QBT API */

    pub fn get_generic_properties(&self) -> Result<GenericInfo> {
        let endpoint = self.client.url("torrents/properties");
        let query = [("hash", &self.info.hash)];
        let resp = self.client.session.get(endpoint).query(&query).send()?;
        let result = resp.json()?;
        Ok(result)
    }

    pub fn get_trackers(&self) {
        let query = [("hash", &self.info.hash)];
        let endpoint = self.client.url("torrents/trackers");
        todo!("Not implemented!");
    }

    pub fn get_webseeds(&self) {
        let query = [("hash", &self.info.hash)];
        let endpoint = self.client.url("torrents/webseeds");
        todo!("Not implemented!");
    }

    pub fn get_contents<C>(&'a self, container: &mut C) -> Result<()>
    where
        C: Extend<Item<'a>>,
    {
        let query = [("hash", &self.info.hash)];
        let endpoint = self.client.url("torrents/files");
        let resp = self.client.session.get(endpoint).query(&query).send()?;

        let mut my_items = resp.json::<Vec<Item>>()?;
        for i in my_items.iter_mut() {
            i.torrent = Some(&self);
        }

        container.extend(my_items);

        Ok(())
    }

    pub fn get_single_item(&'a self, index: u64) -> Result<Item<'a>> {
        let query = [("hash", &self.info.hash), ("indexes", &index.to_string())];
        let endpoint = self.client.url("torrents/files");
        let resp = self.client.session.get(endpoint).query(&query).send()?;
        let item: [Item; 1] = resp.json()?;
        Ok(item[0].clone())
    }

    pub fn get_piece_states(&self) {
        let query = [("hash", &self.info.hash)];
        let endpoint = self.client.url("torrents/pieceStates");
        todo!("Not implemented!");
    }

    pub fn get_piece_hashes(&self) {
        let query = [("hash", &self.info.hash)];
        let endpoint = self.client.url("torrents/pieceHashes");
        todo!("Not implemented!");
    }

    pub fn pause(&self) {
        let query = [("hash", &self.info.hash)];
        let endpoint = self.client.url("torrents/pause");
        todo!("Not implemented!");
    }

    pub fn resume(&self) {
        let query = [("hash", &self.info.hash)];
        let endpoint = self.client.url("torrents/resume");
        todo!("Not implemented!");
    }

    pub fn delete(&self) {
        let query = [("hash", &self.info.hash)];
        let endpoint = self.client.url("torrents/delete");
        todo!("Not implemented!");
    }

    pub fn recheck(&self) {
        let query = [("hash", &self.info.hash)];
        let endpoint = self.client.url("torrents/recheck");
        todo!("Not implemented!");
    }

    pub fn reannounce(&self) {
        let query = [("hash", &self.info.hash)];
        let endpoint = self.client.url("torrents/reannounce");
        todo!("Not implemented!");
    }

    /* Following are additional nice to have features not part of the core API. */

    fn serialize_metadata(&mut self) {
        // File Metadata format for a torrent //
        // 0. name                          - str
        // 1. added_on                      - RFC3339 datetime
        // 2. availability                  - float
        // 3. eta                           - int
        // 4. dl_session downloaded         - int int
        // 5. up_session uploaded           - int int
        // 6. last_activity                 - RFC3339 datetime
        // 7. size                          - int
        // 8. progress                      - float
        // 9. ratio                         - float
        // 10. dl_speed                     - int
        // 11. ul_speed                     - int

        let generic_info = self.get_generic_properties().unwrap();

        // let added_on = DateTime::<Local>::from(self.info.added_on);
        let added_on = Local
            .timestamp_opt(self.info.added_on, 0)
            .unwrap()
            .to_rfc3339();
        let last_activity = Local
            .timestamp_opt(self.info.last_activity, 0)
            .unwrap()
            .to_rfc3339();

        let dl_line = [
            self.info.downloaded_session.to_string(),
            self.info.downloaded.to_string(),
        ]
        .join(" ");
        let ul_line = [
            self.info.uploaded_session.to_string(),
            self.info.uploaded.to_string(),
        ]
        .join(" ");

        let attrs = vec![
            self.info.name.to_string(),
            added_on, //
            self.info.availability.to_string(),
            self.info.eta.to_string(),
            dl_line,
            ul_line,
            last_activity,
            self.info.size.to_string(),
            self.info.progress.to_string(),
            self.info.ratio.to_string(),
            generic_info.dl_speed.to_string(),
            generic_info.up_speed.to_string(),
        ];

        let joined = attrs.join("\n") + "\n";
        let joined_bytes = joined.as_bytes();
        self.metadata_buffer = Some(joined_bytes.to_vec());
    }

    pub fn get_metadata_len(&mut self) -> usize {
        match self.metadata_buffer {
            None => self.serialize_metadata(),
            Some(_) => (),
        }

        let buffer = match &self.metadata_buffer {
            None => return 0,
            Some(x) => return x.len(),
        };
    }

    pub fn get_metadata_bytes(&mut self) -> &[u8] {
        match self.metadata_buffer {
            None => self.serialize_metadata(),
            Some(_) => (),
        }

        let buffer = match &self.metadata_buffer {
            None => return &[],
            Some(x) => return &x,
        };
    }
}

impl<'a> Item<'a> {
    pub fn get_path_components(&self) -> Option<Split<'_, &str>> {
        if !self.name.contains("/") {
            return None;
        }

        Some(self.name.split("/"))
    }
}
