#pragma once

#include "torrent.h"

#include <iostream>
#include <optional>
#include <string>

#include <boost/json/object.hpp>
#include <curl/curl.h>

using std::optional;
using std::string;

struct Credentials
{
  std::string username;
  std::string password;
};

class Qbittorrent
{

public:
  Qbittorrent(const string& baseUrl,
              const optional<Credentials> credentials,
              const bool insecure = false);
  ~Qbittorrent();
  friend class Torrent;

  /* Application ============================================================ */
  std::string getApplicationVersion() const;
  std::string getAPIVersion() const;
  boost::json::object getBuildInfo() const;
  // bool shutdown();
  boost::json::object getApplicationPreferences() const;
  // setApplicationPreferences();
  std::string getDefaultSavePath() const;

  /* Log =================================================================== */
  // getLog();
  // getPeerLog();

  /* Sync ================================================================== */
  // getMainData();
  // getTorrentPeersData();

  /* Transfer Info ========================================================= */
  boost::json::object getGlobalTransferInfo() const;
  bool getAlternativeSpeedLimitsState() const;
  // toggleAlternativeSpeedLimits();
  size_t getGlobalDownloadLimit() const;
  // setGlobalDownloadLimit();
  size_t getGlobalUploadLimit() const;
  // setGlobalUploadLimit();
  // banPeers();

  /* Torrent Management ==================================================== */

  template<typename OutputIterator>
  void getTorrentList(OutputIterator output);

  // getTorrentGenericProperties();
  // getTorrentTrackers();
  // getTorrentWebSeeds();
  // getTorrentContents();
  // getTorrentPiecesStates();
  // getTorrentPiecesHashes();
  // pauseTorrents();
  // resumeTorrents();
  // deleteTorrents();
  // recheckTorrents();
  // reannounceTorrents();
  // editTrackers();
  // removeTrackers();
  // addPeers();
  // addNewTorrent();
  // addTrackersToTorrent();
  // increaseTorrentPriority();
  // decreaseTorrentPriority();
  // maximalTorrentPriority();
  // minimalTorrentPriority();
  // setTorrentPriority();
  // setFilePriority();
  // getTorrentDownloadLimit();
  // setTorrentDownloadLimit();
  // setTorrentLocation();
  // setTorrentName();
  // setTorrentCategory();
  // getAllCategories();
  // addNewCategory();
  // editCategory();
  // removeCategories();
  // addTorrentTags();
  // removeTorrentTags();
  // getAllTags();
  // createTags();
  // deleteTags();
  // setAutomaticTorrentManagement();
  // toggleSequentialDownload();
  // setPiecePriority();
  // setForceStart();
  // setSuperSeeding();
  // renameFile();
  // renameFolder();

  /* RSS (experimental) ==================================================== */
  // (TODO)

  /* Search ================================================================ */
  // (TODO)

private:
  std::string buildUrl(const std::string& endpoint) const;

  /* ======================================================================= */
  /* Core ================================================================== */
  void init();
  bool login();
  void logout();

  std::string getGenericStr(const std::string& endpoint) const;
  boost::json::object getGenericJson(const std::string& endpoint) const;

  const bool insecure;
  const string baseUrl;
  const optional<Credentials> credentials;
  const optional<string> appPassword;
  struct curl_slist* headers;

  CURL* curl;
};
