
#include "torrent.h"
#include "../logger.h"
#include "core.h"
#include "web.h"

#include <cstdint>
#include <ctime>
#include <string>
#include <unordered_map>

#include <boost/date_time.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/json.hpp>
#include <boost/json/array.hpp>
#include <boost/json/object.hpp>
#include <boost/json/parse.hpp>
#include <boost/json/serialize.hpp>
#include <boost/json/value.hpp>
#include <boost/json/value_to.hpp>
#include <boost/log/trivial.hpp>
#include <curl/curl.h>
#include <curl/easy.h>
#include <fmt/core.h>
#include <fmt/format.h>

namespace json = boost::json;
namespace chrono = std::chrono;
namespace posix_time = boost::posix_time;

Torrent::ContentItem::ContentItem(const boost::json::object& obj)
{
  // clang-format off
  index        = json::value_to<int64_t>(obj.at("index"));
  name         = json::value_to<std::string>(obj.at("name"));
  size         = json::value_to<size_t>(obj.at("size"));
  progress     = json::value_to<float>(obj.at("progress"));
  priority     = json::value_to<int>(obj.at("priority"));
  availability = json::value_to<float>(obj.at("availability"));
  // clang-format on

  // Sometimes is_seed is missing.
  if (obj.find("is_seed") != obj.end()) {
    isSeed = json::value_to<bool>(obj.at("is_seed"));
  } else {
    isSeed = false;
  }
}

const Torrent::State
strToState(const std::string& str)
{
  if (str == "error") {
    return Torrent::error;
  } else if (str == "missingFiles") {
    return Torrent::missingFiles;
  } else if (str == "uploading") {
    return Torrent::uploading;
  } else if (str == "pausedUP") {
    return Torrent::pausedUP;
  } else if (str == "queuedUP") {
    return Torrent::queuedUP;
  } else if (str == "stalledUP") {
    return Torrent::stalledUP;
  } else if (str == "checkingUP") {
    return Torrent::checkingUP;
  } else if (str == "forcedUP") {
    return Torrent::forcedUP;
  } else if (str == "allocating") {
    return Torrent::allocating;
  } else if (str == "downloading") {
    return Torrent::downloading;
  } else if (str == "metaDL") {
    return Torrent::metaDL;
  } else if (str == "pausedDL") {
    return Torrent::pausedDL;
  } else if (str == "stalledDL") {
    return Torrent::stalledDL;
  } else if (str == "checkingDL") {
    return Torrent::checkingDL;
  } else if (str == "forcedDL") {
    return Torrent::forcedDL;
  } else if (str == "checkingResumeData") {
    return Torrent::checkingResumeData;
  } else if (str == "moving") {
    return Torrent::moving;
  }
  return Torrent::unknown;
}

Torrent::Torrent(const Qbittorrent* parent, const json::object& obj)
  : parent(parent)
{
  time_t epoch;

  // clang-format off
  epoch                = json::value_to<time_t>(obj.at("added_on"));
  addedOn              = posix_time::from_time_t(epoch);
  amountLeft           = json::value_to<long>(obj.at("amount_left"));
  autoTMM              = json::value_to<bool>(obj.at("auto_tmm"));
  availability         = json::value_to<float>(obj.at("availability"));
  category             = json::value_to<std::string>(obj.at("category"));
  completed            = json::value_to<size_t>(obj.at("completed"));
  epoch                = json::value_to<time_t>(obj.at("completion_on"));
  completionOn         = posix_time::from_time_t(epoch);
  contentPath          = json::value_to<std::string>(obj.at("content_path"));
  dlLimit              = json::value_to<int64_t>(obj.at("dl_limit"));
  dlSpeed              = json::value_to<int64_t>(obj.at("dlspeed"));
  downloaded           = json::value_to<int64_t>(obj.at("downloaded"));
  downloadedSession    = json::value_to<int64_t>(obj.at("downloaded_session"));
  eta                  = json::value_to<int64_t>(obj.at("eta"));
  flPiecePrio          = json::value_to<bool>(obj.at("f_l_piece_prio"));
  forceStart           = json::value_to<bool>(obj.at("force_start"));
  hash                 = json::value_to<std::string>(obj.at("hash"));
  isPrivate            = json::value_to<bool>(obj.at("private"));
  epoch                = json::value_to<time_t>(obj.at("last_activity"));
  lastActivity         = posix_time::from_time_t(epoch);
  magnetUri            = json::value_to<std::string>(obj.at("magnet_uri"));
  maxRatio             = json::value_to<float>(obj.at("max_ratio"));
  maxSeedingTime       = json::value_to<int64_t>(obj.at("max_seeding_time"));
  name                 = json::value_to<std::string>(obj.at("name"));
  numComplete          = json::value_to<long>(obj.at("num_complete"));
  numIncomplete        = json::value_to<long>(obj.at("num_incomplete"));
  numLeechs            = json::value_to<long>(obj.at("num_leechs"));
  numSeeds             = json::value_to<long>(obj.at("num_seeds"));
  priority             = json::value_to<long>(obj.at("priority"));
  progress             = json::value_to<float>(obj.at("progress"));
  ratio                = json::value_to<float>(obj.at("ratio"));
  ratioLimit           = json::value_to<float>(obj.at("ratio_limit"));
  savePath             = json::value_to<std::string>(obj.at("save_path"));
  seedingTime          = json::value_to<long>(obj.at("seeding_time"));
  seedingTimeLimit     = json::value_to<long>(obj.at("seeding_time_limit"));
  epoch                = json::value_to<time_t>(obj.at("seen_complete"));
  seenComplete         = posix_time::from_time_t(epoch);
  seqDl                = json::value_to<bool>(obj.at("seq_dl"));
  size                 = json::value_to<size_t>(obj.at("size"));
  state                = strToState(json::value_to<std ::string>(obj.at("state")));
  superSeeding         = json::value_to<bool>(obj.at("super_seeding"));
  tags                 = json::value_to<std::string>(obj.at("tags"));
  timeActive           = json::value_to<long>(obj.at("time_active"));
  totalSize            = json::value_to<size_t>(obj.at("total_size"));
  tracker              = json::value_to<std::string>(obj.at("tracker"));
  upLimit              = json::value_to<int64_t>(obj.at("up_limit"));
  uploaded             = json::value_to<int64_t>(obj.at("uploaded"));
  uploadedSession      = json::value_to<int64_t>(obj.at("uploaded_session"));
  upSpeed              = json::value_to<int64_t>(obj.at("upspeed"));

  // clang-format on
}

Torrent::GenericInfo&
Torrent::getGenericInfo(bool force)
{
  if (genericInfo.has_value() && !force) {
    return genericInfo.value();
  }

  const std::string endpoint =
    parent->buildUrl(fmt::format("torrents/properties?hash={}", hash));
  std::string buffer;
  CURLcode res = GET(parent->curl, endpoint, &buffer);
  if (res != CURLE_OK) {
    const std::string err = curl_easy_strerror(res);
    LOG_LOC(fatal) << err;
  }
  boost::json::value jvp = json::parse(buffer);
  boost::json::object obj = jvp.as_object();

  time_t epoch;
  GenericInfo info;
  /* Generic Info */
  // clang-format off
  info.savePath             = json::value_to<std::string>(obj.at("save_path"));
  epoch                     = json::value_to<time_t>(obj.at("creation_date"));
  info.creationDate         = posix_time::from_time_t(epoch);
  info.pieceSize            = json::value_to<long>(obj.at("piece_size"));
  info.comment              = json::value_to<std::string>(obj.at("comment"));
  info.totalWasted          = json::value_to<size_t>(obj.at("total_wasted"));
  info.totalUploaded        = json::value_to<size_t>(obj.at("total_uploaded"));
  info.totalUploadedSession = json::value_to<size_t>(obj.at("total_uploaded_session"));
  info.upLimit              = json::value_to<int64_t>(obj.at("up_limit"));
  info.dlLimit              = json::value_to<int64_t>(obj.at("dl_limit"));
  info.timeElapsed          = json::value_to<int64_t>(obj.at("time_elapsed"));
  info.seedingTime          = json::value_to<int64_t>(obj.at("seeding_time"));
  info.nbConnections        = json::value_to<long>(obj.at("nb_connections"));
  info.nbConnectionsLimit   = json::value_to<long>(obj.at("nb_connections_limit"));
  info.shareRatio           = json::value_to<float>(obj.at("share_ratio"));
  epoch                     = json::value_to<time_t>(obj.at("addition_date"));
  info.additionDate         = posix_time::from_time_t(epoch);
  epoch                     = json::value_to<time_t>(obj.at("completion_date"));
  info.completionDate       = posix_time::from_time_t(epoch);
  info.createdBy            = json::value_to<std::string>(obj.at("created_by"));
  info.dlSpeedAvg           = json::value_to<int64_t>(obj.at("dl_speed_avg"));
  info.dlSpeed              = json::value_to<int64_t>(obj.at("dl_speed"));
  info.eta                  = json::value_to<int64_t>(obj.at("eta"));
  epoch                     = json::value_to<time_t>(obj.at("last_seen"));
  info.lastSeen             = posix_time::from_time_t(epoch);
  info.peers                = json::value_to<long>(obj.at("peers"));
  info.peersTotal           = json::value_to<long>(obj.at("peers_total"));
  info.piecesHave           = json::value_to<int64_t>(obj.at("pieces_have"));
  info.piecesNum            = json::value_to<int64_t>(obj.at("pieces_num"));
  info.reannounce           = json::value_to<long>(obj.at("reannounce"));
  info.seeds                = json::value_to<long>(obj.at("seeds"));
  info.seedsTotal           = json::value_to<long>(obj.at("seeds_total"));
  info.totalSize            = json::value_to<size_t>(obj.at("total_size"));
  info.upSpeed              = json::value_to<int64_t>(obj.at("up_speed"));
  info.upSpeedAvg           = json::value_to<int64_t>(obj.at("up_speed_avg"));
  // clang-format on

  genericInfo.emplace(info);

  return genericInfo.value();
}

auto
fmt::formatter<Torrent::State>::format(const Torrent::State& s,
                                       format_context& ctx) const
  -> format_context::iterator
{
  std::string_view str = STATE_STRS.at(s);
  return formatter<string_view>::format(str, ctx);
}

auto
fmt::formatter<Torrent::ContentItem>::format(const Torrent::ContentItem& c,
                                             format_context& ctx) const
  -> format_context::iterator
{
  const std::string_view s = fmt::format("TODO: {}:{}", __FILE__, __LINE__);
  return formatter<string_view>::format(s, ctx);
}
