use anyhow::Result;
use serde::{Deserialize, Serialize};

use crate::qbt::core::Client;

#[derive(Serialize, Deserialize, Debug)]
pub enum ConnectionStatus {
    #[serde(rename = "connected")]
    Connected,
    #[serde(rename = "firewalled")]
    Firewalled,
    #[serde(rename = "disconnected")]
    Disconnected,
}

#[derive(Serialize, Deserialize, Debug)]
pub struct TransferInfo {
    /// Global download rate (bytes/s)
    dl_info_speed: i64,
    /// Data downloaded this session (bytes)
    dl_info_data: i64,
    /// Global upload rate (bytes/s)
    up_info_speed: i64,
    /// Data uploaded this session (bytes)
    up_info_data: i64,
    /// Download rate limit (bytes/s)
    dl_rate_limit: i64,
    /// Upload rate limit (bytes/s)
    up_rate_limit: i64,
    /// DHT nodes connected to
    dht_nodes: i64,
    /// connection status
    connection_status: ConnectionStatus,
}

impl Client {
    pub fn get_global_transfer_info(&self) -> Result<TransferInfo> {
        let endpoint = self.url("transfer/info");
        let resp = self.session.get(endpoint).send()?;
        let result: TransferInfo = resp.json()?;
        Ok(result)
    }

    pub fn alternative_speed_limits_enabled(&self) -> Result<bool> {
        let endpoint = self.url("transfer/speedLimitsMode");
        let resp = self.session.get(endpoint).send()?;
        let result = match resp.text()?.as_str() {
            "1" => true,
            _ => false,
        };
        Ok(result)
    }

    pub fn toggle_alternative_speed_limits(&self) {
        todo!("Not implemented!");
    }

    pub fn get_global_download_limit(&self) -> Result<u64> {
        let endpoint = self.url("transfer/downloadLimit");
        let resp = self.session.get(endpoint).send()?;
        let result = resp.text()?.parse::<u64>()?;
        Ok(result)
    }

    pub fn set_global_download_limit(&self) {
        todo!("Not implemented!");
    }

    pub fn get_global_upload_limit(&self) -> Result<u64> {
        let endpoint = self.url("transfer/uploadLimit");
        let resp = self.session.get(endpoint).send()?;
        let result = resp.text()?.parse::<u64>()?;
        Ok(result)
    }

    pub fn set_global_upload_limit(&self) {
        todo!("Not implemented!");
    }
}
