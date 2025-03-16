use anyhow::{bail, Context, Result};
use reqwest::{
    self,
    header::{self, HeaderMap},
};
use serde::{Deserialize, Serialize};

#[derive(Serialize, Deserialize, Debug)]
pub(super) struct Credentials {
    username: String,
    password: String,
}

#[derive(Debug)]
pub struct Client {
    pub(super) base_url: String,
    pub(super) credentials: Credentials,
    pub(super) session: reqwest::blocking::Client,
}

impl Client {
    pub(super) fn url(&self, endpoint: &str) -> String {
        format!("{}/{}", self.base_url, endpoint)
    }

    pub fn new(base_url: &str, username: &str, password: &str, ssl_verify: bool) -> Result<Client> {
        let mut headers = HeaderMap::new();
        headers.append("Referer", header::HeaderValue::from_str(base_url)?);

        let session = reqwest::blocking::ClientBuilder::new()
            .danger_accept_invalid_certs(!ssl_verify)
            .default_headers(headers)
            .cookie_store(true)
            .build()?;
        let base_url = format!("{}/api/v2", base_url);
        Ok(Self {
            base_url,
            credentials: Credentials {
                username: username.to_string(),
                password: password.to_string(),
            },
            session,
        })
    }

    pub fn login(&mut self) -> Result<()> {
        let endpoint = self.url("auth/login");
        let resp = self.session.post(endpoint).form(&self.credentials).send()?;

        println!("Response code: {}", resp.status());

        match resp.error_for_status_ref() {
            Ok(_) => Ok(()),
            Err(e) => {
                let status = e.status();
                if status.is_some_and(|x| x == 403) {
                    Err(e).with_context(|| "User's IP is banned for too many failed login attempts")
                } else {
                    Err(e).with_context(|| "Failed to login")
                }
            }
        }?;

        let text = resp.text()?;
        if text == "Fails." {
            bail!("{}: Invalid credentials", text)
        }

        Ok(())
    }

    pub fn logout(&mut self) -> Result<()> {
        let endpoint = self.url("auth/logout");
        self.session.post(endpoint).send()?;

        Ok(())
    }
}
