use anyhow::Result;
use serde::{Deserialize, Serialize};

use crate::qbt::core::Client;

#[derive(Serialize, Deserialize, Debug)]
pub struct BuildInfo {
    qt: String,
    libtorrent: String,
    boost: String,
    openssl: String,
    bitness: u32,
}

impl Client {
    pub fn get_version(&self) -> Result<String> {
        let endpoint = self.url("app/version");
        let resp = self.session.get(endpoint).send()?;
        Ok(resp.text()?)
    }

    pub fn get_api_version(&self) -> Result<String> {
        let endpoint = self.url("app/webapiVersion");
        let resp = self.session.get(endpoint).send()?;
        Ok(resp.text()?)
    }

    pub fn get_build_info(&self) -> Result<BuildInfo> {
        let endpoint = self.url("app/buildInfo");
        let resp = self.session.get(endpoint).send()?;
        let result: BuildInfo = resp.json()?;
        Ok(result)
    }

    // TODO:
    //   * shutdown
    //   * preferences
    //   * setPreferences

    pub fn get_default_save_path(&self) -> Result<String> {
        let endpoint = self.url("app/defaultSavePath");
        let resp = self.session.get(endpoint).send()?;
        Ok(resp.text()?)
    }
}
