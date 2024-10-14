#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>

#include <boost/date_time.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/json/object.hpp>
#include <fmt/core.h>
#include <fmt/format.h>

class Qbittorrent;

class Torrent
{

public:
  Torrent(const Qbittorrent* parent, const boost::json::object& obj);
  ~Torrent() = default;

  enum State
  {
    error = 0,
    missingFiles,
    uploading,
    pausedUP,
    queuedUP,
    stalledUP,
    checkingUP,
    forcedUP,
    allocating,
    downloading,
    metaDL,
    pausedDL,
    queuedDL,
    stalledDL,
    checkingDL,
    forcedDL,
    checkingResumeData,
    moving,
    unknown,
  };

  struct GenericInfo
  {
    std::string savePath;
    boost::posix_time::ptime creationDate;
    long pieceSize;
    std::string comment;
    size_t totalWasted;
    size_t totalUploaded;
    size_t totalUploadedSession;
    size_t totalDownloaded;
    size_t totalDownloadedSession;
    int64_t upLimit;
    int64_t dlLimit;
    int64_t timeElapsed;
    int64_t seedingTime;
    long nbConnections;
    long nbConnectionsLimit;
    float shareRatio;
    boost::posix_time::ptime additionDate;
    boost::posix_time::ptime completionDate;
    std::string createdBy;
    int64_t dlSpeedAvg;
    int64_t dlSpeed;
    int64_t eta;
    boost::posix_time::ptime lastSeen;
    long peers;
    long peersTotal;
    int64_t piecesHave;
    int64_t piecesNum;
    long reannounce;
    long seeds;
    long seedsTotal;
    size_t totalSize;
    int64_t upSpeedAvg;
    int64_t upSpeed;
  };

  class ContentItem
  {
  public:
    ContentItem(const boost::json::object& obj);
    int64_t index;
    std::string name;
    size_t size;
    float progress;
    int priority;
    bool isSeed;
    // TODO:
    // std::vector<int> pieceRange;
    float availability;
  };

  boost::posix_time::ptime addedOn;
  long amountLeft;
  bool autoTMM;
  float availability;
  std::string category;
  size_t completed;
  boost::posix_time::ptime completionOn;
  std::string contentPath;

  int64_t dlLimit;
  int64_t dlSpeed;
  int64_t downloaded;
  int64_t downloadedSession;
  int64_t eta;
  bool flPiecePrio;
  bool forceStart;
  std::string hash;
  bool isPrivate;
  boost::posix_time::ptime lastActivity;
  std::string magnetUri;
  float maxRatio;
  int64_t maxSeedingTime;
  std::string name;
  long numComplete;
  long numIncomplete;
  long numLeechs;
  long numSeeds;
  long priority;
  float progress;
  float ratio;
  float ratioLimit;
  std::string savePath;
  long seedingTime;
  long seedingTimeLimit;
  boost::posix_time::ptime seenComplete;
  bool seqDl;
  size_t size;
  State state;
  bool superSeeding;
  std::string tags;
  long timeActive;
  size_t totalSize;
  std::string tracker;
  int64_t upLimit;
  int64_t uploaded;
  int64_t uploadedSession;
  int64_t upSpeed;

  Torrent::GenericInfo& getGenericInfo(bool force = false);

  template<typename OutputIterator>
  void getContents(OutputIterator output);

private:
  std::optional<Torrent::GenericInfo> genericInfo;
  const Qbittorrent* parent;
};

const std::unordered_map<Torrent::State, const char*> STATE_STRS = {
  { Torrent::error, "Error" },
  { Torrent::missingFiles, "Missing Files" },
  { Torrent::uploading, "Uploading" },
  { Torrent::pausedUP, "Paused Uploading" },
  { Torrent::pausedDL, "Paused Downloading" },
  { Torrent::queuedUP, "Queued Uploading" },
  { Torrent::stalledUP, "Stalled Uploading" },
  { Torrent::checkingUP, "Checking Uploading" },
  { Torrent::forcedUP, "Forced Uploading" },
  { Torrent::allocating, "Allocating Space" },
  { Torrent::downloading, "Downloading" },
  { Torrent::metaDL, "Metadata Downloading" },
  { Torrent::pausedDL, "Paused Downloading" },
  { Torrent::queuedDL, "Queued Download" },
  { Torrent::stalledDL, "Stalled Download" },
  { Torrent::checkingDL, "Checking Download" },
  { Torrent::forcedDL, "Forced Downloading" },
  { Torrent::checkingResumeData, "Checking Resume Data" },
  { Torrent::moving, "Moving" },
  { Torrent::unknown, "Unkown" },
};

const Torrent::State
strToState(const std::string& str);

template<>
struct fmt::formatter<Torrent::State> : formatter<string_view>
{
  auto format(const Torrent::State& s, format_context& ctx) const
    -> format_context::iterator;
};

template<>
struct fmt::formatter<Torrent::ContentItem> : formatter<string_view>
{
  auto format(const Torrent::ContentItem& c, format_context& ctx) const
    -> format_context::iterator;
};
