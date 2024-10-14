#include "../logger.h"
#include "qbt.h"
#include "web.h"

#include <fmt/core.h>
#include <optional>
#include <stdexcept>
#include <string>

#include <boost/json/object.hpp>
#include <boost/json/parse.hpp>
#include <boost/json/serialize.hpp>
#include <boost/json/value.hpp>
#include <boost/log/trivial.hpp>
#include <curl/curl.h>
#include <curl/easy.h>

#include <ios>
#include <iosfwd>

using std::optional;
using std::string;
namespace json = boost::json;

Qbittorrent::Qbittorrent(const string& baseUrl,
                         const optional<Credentials> credentials,
                         const bool insecure)
  : baseUrl(baseUrl)
  , credentials(credentials)
  , insecure(insecure)
  , headers(nullptr)
{
  init();
}

Qbittorrent::~Qbittorrent()
{
  logout();
  curl_easy_cleanup(curl);
  curl_slist_free_all(headers);
  curl_global_cleanup();
}

std::string
Qbittorrent::buildUrl(const std::string& endpoint) const
{
  return fmt::format("{}/api/v2/{}", baseUrl, endpoint);
}

void
Qbittorrent::init()
{
  // Setup the CURL session.
  curl = nullptr;
  curl_global_init(CURL_GLOBAL_ALL);

  curl = curl_easy_init();
  if (!curl) {
    throw std::runtime_error("Failed to initialize CURL session.");
  }

  if (insecure) {
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
  }

  curl_easy_setopt(curl, CURLOPT_CA_CACHE_TIMEOUT, 604800L);
  curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
  curl_easy_setopt(curl, CURLOPT_VERBOSE, 0L);

  // Setup the headers
  if (appPassword.has_value()) {
    const std::string header =
      fmt::format("Authorization: Bearer {}", appPassword.value());

    headers = curl_slist_append(headers, header.c_str());
  }

  if (headers) {
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
  }

  curl_easy_setopt(curl, CURLOPT_URL, baseUrl.c_str());
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curlStrWriteCallback);

  if (!login()) {
    throw std::runtime_error("Authentication failed");
  }
}

bool
Qbittorrent::login()
{
  if (!credentials.has_value()) {
    LOG_LOC(warning) << "Credentials not provided.";
  }

  // Assemble the POST JSON
  json::object creds;
  const Credentials dummy{};
  creds["username"] = credentials.value_or(dummy).username;
  creds["password"] = credentials.value_or(dummy).password;
  std::string credsJson = json::serialize(creds);

  std::string resp;
  const std::string endpoint = buildUrl("auth/login");
  LOG_LOC(debug) << "Login: " << endpoint;

  CURLcode res = POST(curl, endpoint, credsJson, &resp);
  if (res != CURLE_OK) {
    const std::string err = curl_easy_strerror(res);
    LOG_LOC(fatal) << err;
    return false;
  }

  LOG_LOC(debug) << "Authenticate: " << resp;

  // TODO: Test more
  if (resp != "Ok.") {
    LOG_LOC(fatal) << "Failed to authenticate to QBT";
    return false;
  }
  return true;
}

void
Qbittorrent::logout()
{
  std::string resp;
  const std::string endpoint = buildUrl("auth/logout");
  LOG_LOC(debug) << "Logout: " << endpoint;

  CURLcode res = POST(curl, endpoint, "", &resp);
  if (res != CURLE_OK) {
    const std::string err = curl_easy_strerror(res);
    LOG_LOC(fatal) << err;
  }
}

std::string
Qbittorrent::getGenericStr(const std::string& endpoint) const
{
  std::string resp;
  CURLcode res = GET(curl, endpoint, &resp);
  if (res != CURLE_OK) {
    const std::string err = curl_easy_strerror(res);
    LOG_LOC(fatal) << err;
  }

  return resp;
}

json::object
Qbittorrent::getGenericJson(const std::string& endpoint) const
{
  std::string resp;
  CURLcode res = GET(curl, endpoint, &resp);
  if (res != CURLE_OK) {
    const std::string err = curl_easy_strerror(res);
    LOG_LOC(fatal) << err;
  }

  // Parse the JSON
  json::value jv = json::parse(resp);
  return jv.as_object();
}

/* ========================================================================= */
/* Application ============================================================= */
/* ========================================================================= */

std::string
Qbittorrent::getApplicationVersion() const
{
  const std::string endpoint = buildUrl("app/version");
  LOG_LOC(debug) << "Get Application Version: " << endpoint;
  return getGenericStr(endpoint);
}

std::string
Qbittorrent::getAPIVersion() const
{
  std::string resp;
  const std::string endpoint = buildUrl("app/webapiVersion");
  LOG_LOC(debug) << "Get API Version: " << endpoint;
  return getGenericStr(endpoint);
}

json::object
Qbittorrent::getBuildInfo() const
{
  const std::string endpoint = buildUrl("app/buildInfo");
  LOG_LOC(debug) << "Get Build Info: " << endpoint;
  return getGenericJson(endpoint);
}

json::object
Qbittorrent::getApplicationPreferences() const
{
  const std::string endpoint = buildUrl("app/preferences");
  LOG_LOC(debug) << "Get Application Preferences: " << endpoint;
  return getGenericJson(endpoint);
}

std::string
Qbittorrent::getDefaultSavePath() const
{
  const std::string endpoint = buildUrl("app/defaultSavePath");
  LOG_LOC(debug) << "Get Default Save Path: " << endpoint;
  return getGenericStr(endpoint);
}

/* ========================================================================= */
/* Log ===================================================================== */
/* ========================================================================= */

// (TODO)

/* ========================================================================= */
/* Sync ==================================================================== */
/* ========================================================================= */

// (TODO)

/* ========================================================================= */
/* Transfer Info =========================================================== */
/* ========================================================================= */

json::object
Qbittorrent::getGlobalTransferInfo() const
{
  const std::string endpoint = buildUrl("transfer/info");
  LOG_LOC(debug) << "Get global transfer info: " << endpoint;
  return getGenericJson(endpoint);
}

bool
Qbittorrent::getAlternativeSpeedLimitsState() const
{
  const std::string endpoint = buildUrl("transfer/speedLimitsMode");
  LOG_LOC(debug) << "Get speed limits mode: " << endpoint;
  std::string resp = getGenericStr(endpoint);

  std::istringstream is(resp);
  bool res;
  is >> std::boolalpha >> res;
  return res;
}

size_t
Qbittorrent::getGlobalDownloadLimit() const
{
  const std::string endpoint = buildUrl("transfer/downloadLimit");
  LOG_LOC(debug) << "Get global download limit: " << endpoint;
  std::string resp = getGenericStr(endpoint);

  std::istringstream is(resp);
  size_t res;
  is >> res;
  return res;
}

size_t
Qbittorrent::getGlobalUploadLimit() const
{
  const std::string endpoint = buildUrl("transfer/uploadLimit");
  LOG_LOC(debug) << "Get global upload limit: " << endpoint;
  std::string resp = getGenericStr(endpoint);

  std::istringstream is(resp);
  size_t res;
  is >> res;
  return res;
}

/* ========================================================================= */
/* Torrent Management ====================================================== */
/* ========================================================================= */

/*
template<template<typename, typename> typename Container, typename Allocator>
void
Qbittorrent::getTorrentList(Container<Torrent, Allocator>& args)
{
  const std::string endpoint = buildUrl("torrents/info");
  LOG_LOC(debug) << "Get torrent list: " << endpoint;
  json::value resp = getGenericJson(endpoint);
  std::cout << "foobar" << std::endl;
}
*/

// (TODO)

/* ========================================================================= */
/* RSS ===================================================================== */
/* ========================================================================= */

// (TODO)

/* ========================================================================= */
/* Search ================================================================== */
/* ========================================================================= */

// (TODO)
