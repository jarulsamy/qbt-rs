
#include "debug.h"
#include "fs/fs.h"
#include "logger.h"
#include "qbt/qbt.h"
#include "qbt/shell.h"
#include "tui/tui.h"

#include <exception>
#include <fmt/core.h>
#include <string>
#include <vector>

#include "boost/json.hpp"
#include "boost/json/object.hpp"
#include <boost/program_options.hpp>

namespace po = boost::program_options;
namespace logging = boost::log;
namespace posix_time = boost::posix_time;

void
debugQbt(bool insecure);

void
debugShell();

void
debugFS();

int
main(int argc, char* argv[])
{
  // Setup CLI parser
  po::options_description desc("Options");
  // clang-format off
  desc.add_options()
    ("help,h", "Display help info")
    (
      "verbose,v",
      po::value<unsigned>()->implicit_value(1)->default_value(0),
      "Increase logging levels."
    )
    (
      "insecure,k",
      "Disable SSL verification."
    );
  // clang-format on

  po::variables_map vm;
  try {
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);
  } catch (std::exception& e) {
    fmt::print("Error: {}", e.what());
    std::cout << desc << "\n";
    return 1;
  }

  if (vm.count("help")) {
    std::cout << desc << "\n";
    return 0;
  }

  // Setup logger
  setupLogger(vm["verbose"].as<unsigned>());

  debugQbt(vm.count("insecure"));
  // debugShell();
  // debugFS();

  return 0;
}

void
debugQbt(bool insecure)
{
  Qbittorrent qbt(
    /* baseUrl = */ "https://qbt.oasis.arulsamy.me",
    Credentials{
      /* username = */ "USERNAME",
      /* password = */ "PASSWORD",
    },
    /* insecure = */ insecure);

  const std::string appVersion = qbt.getApplicationVersion();
  const std::string apiVersion = qbt.getAPIVersion();
  const boost::json::object buildInfo = qbt.getBuildInfo();
  const boost::json::object appPrefs = qbt.getApplicationPreferences();
  const std::string savePath = qbt.getDefaultSavePath();

  const boost::json::object globalTransferInfo = qbt.getGlobalTransferInfo();
  const bool speedLimitMode = qbt.getAlternativeSpeedLimitsState();
  const size_t globalDownloadLimit = qbt.getGlobalDownloadLimit();
  const size_t globalUploadLimit = qbt.getGlobalUploadLimit();

  std::vector<Torrent> torrents;
  qbt.getTorrentList(std::back_inserter(torrents));

  fmt::print("appVersion: {}\n", appVersion);
  fmt::print("apiVersion: {}\n", apiVersion);
  fmt::print("buildInfo:\n");
  DebugTools::json_pretty_print(std::cout, buildInfo);
  fmt::print("appPrefs:\n");
  DebugTools::json_pretty_print(std::cout, appPrefs);
  fmt::print("savePath:   {}\n", savePath);

  fmt::print("globalTransferInfo:\n");
  DebugTools::json_pretty_print(std::cout, globalTransferInfo);
  fmt::print("speedLimitMode:      {}\n", speedLimitMode);
  fmt::print("globalDownloadLimit: {}\n", globalDownloadLimit);
  fmt::print("globalUploadLImit:   {}\n", globalUploadLimit);

  for (Torrent& t : torrents) {
    fmt::print("==== name: {}\n", t.name);
    fmt::print("  Added on: {}\n",
               posix_time::to_iso_extended_string(t.addedOn));
    fmt::print("  amountLeft: {}\n", t.amountLeft);
    fmt::print("  autoTMM: {}\n", t.autoTMM);
    fmt::print("  availability: {}\n", t.availability);
    fmt::print("  category: {}\n", t.category);
    fmt::print("  completed: {}\n", t.completed);
    fmt::print("  completionOn: {}\n",
               posix_time::to_iso_extended_string(t.completionOn));
    fmt::print("  contentPath: {}\n", t.contentPath);
    fmt::print("  dlLimit: {}\n", t.dlLimit);
    fmt::print("  dlSpeed: {}\n", t.dlSpeed);
    fmt::print("  downloaded: {}\n", t.downloaded);
    fmt::print("  downloadedSession: {}\n", t.downloadedSession);
    fmt::print("  eta: {}\n", t.eta);
    fmt::print("  flPiecePrio: {}\n", t.flPiecePrio);
    fmt::print("  forceStart: {}\n", t.forceStart);
    fmt::print("  hash: {}\n", t.hash);
    fmt::print("  isPrivate: {}\n", t.isPrivate);
    fmt::print("  lastActivity: {}\n",
               posix_time::to_iso_extended_string(t.lastActivity));
    fmt::print("  magnetUri: {}\n", t.magnetUri);
    fmt::print("  maxRatio: {}\n", t.maxRatio);
    fmt::print("  maxSeedingTime: {}\n", t.maxSeedingTime);
    fmt::print("  numComplete: {}\n", t.numComplete);
    fmt::print("  numIncomplete: {}\n", t.numIncomplete);
    fmt::print("  numLeechs: {}\n", t.numLeechs);
    fmt::print("  numSeeds: {}\n", t.numSeeds);
    fmt::print("  priority: {}\n", t.priority);
    fmt::print("  progress: {}\n", t.progress);
    fmt::print("  ratio: {}\n", t.ratio);
    fmt::print("  ratioLimit: {}\n", t.ratioLimit);
    fmt::print("  savePath: {}\n", t.savePath);
    fmt::print("  seedingTime: {}\n", t.seedingTime);
    fmt::print("  seedingTimeLimit: {}\n", t.seedingTimeLimit);
    fmt::print("  seenComplete: {}\n",
               posix_time::to_iso_extended_string(t.seenComplete));
    fmt::print("  seqDl: {}\n", t.seqDl);
    fmt::print("  size: {}\n", t.size);
    fmt::print("  state: {}\n", t.state);
    fmt::print("  superSeeding: {}\n", t.superSeeding);
    fmt::print("  tags: {}\n", t.tags);
    fmt::print("  timeActive: {}\n", t.timeActive);
    fmt::print("  totalSize: {}\n", t.totalSize);
    fmt::print("  tracker: {}\n", t.tracker);
    fmt::print("  upLimit: {}\n", t.upLimit);
    fmt::print("  uploaded: {}\n", t.uploaded);
    fmt::print("  uploadedSession: {}\n", t.uploadedSession);
    fmt::print("  upSpeed: {}\n", t.upSpeed);
    fmt::print("  ======================\n");
    auto info = t.getGenericInfo();
    fmt::print("    savePath: {}\n", info.savePath);
    fmt::print("    creationDate: {}\n",
               posix_time::to_iso_extended_string(info.creationDate));
    fmt::print("    pieceSize: {}\n", info.pieceSize);
    fmt::print("    comment: {}\n", info.comment);
    fmt::print("    totalWasted: {}\n", info.totalWasted);
    fmt::print("    totalUploaded: {}\n", info.totalUploaded);
    fmt::print("    totalUploadedSession: {}\n", info.totalUploadedSession);
    fmt::print("    upLimit: {}\n", info.upLimit);
    fmt::print("    dlLimit: {}\n", info.dlLimit);
    fmt::print("    timeElapsed: {}\n", info.timeElapsed);
    fmt::print("    seedingTime: {}\n", info.seedingTime);
    fmt::print("    nbConnections: {}\n", info.nbConnections);
    fmt::print("    nbConnectionsLimit: {}\n", info.nbConnectionsLimit);
    fmt::print("    shareRatio: {}\n", info.shareRatio);
    fmt::print("    additionDate: {}\n",
               posix_time::to_iso_extended_string(info.additionDate));
    fmt::print("    completionDate: {}\n",
               posix_time::to_iso_extended_string(info.completionDate));
    fmt::print("    createdBy: {}\n", info.createdBy);
    fmt::print("    dlSpeedAvg: {}\n", info.dlSpeedAvg);
    fmt::print("    dlSpeed: {}\n", info.dlSpeed);
    fmt::print("    eta: {}\n", info.eta);
    fmt::print("    lastSeen: {}\n",
               posix_time::to_iso_extended_string(info.lastSeen));
    fmt::print("    peers: {}\n", info.peers);
    fmt::print("    peersTotal: {}\n", info.peersTotal);
    fmt::print("    piecesHave: {}\n", info.piecesHave);
    fmt::print("    piecesNum: {}\n", info.piecesNum);
    fmt::print("    reannounce: {}\n", info.reannounce);
    fmt::print("    seeds: {}\n", info.seeds);
    fmt::print("    seedsTotal: {}\n", info.seedsTotal);
    fmt::print("    totalSize: {}\n", info.totalSize);
    fmt::print("    upSpeed: {}\n", info.upSpeed);
    fmt::print("    upSpeedAvg: {}\n", info.upSpeedAvg);

    std::vector<Torrent::ContentItem> contents;
    t.getContents(std::back_inserter(contents));
    fmt::print("=== CONTENTS =====================\n");
    for (auto& c : contents) {
      fmt::print("===========\n");
      fmt::print("     index: {}\n", c.index);
      fmt::print("     name: {}\n", c.name);
      fmt::print("     size: {}\n", c.size);
      fmt::print("     progress: {}\n", c.progress);
      fmt::print("     priority: {}\n", c.priority);
      fmt::print("     isSeed: {}\n", c.isSeed);
      fmt::print("     availability: {}\n", c.availability);
    }

    contents.clear();
  }
}

void
debugShell()
{
  std::cout << "/////////////////////////////////////////////////////////\n\n";
  std::cout << "\t\tA Space separated list parser for Spirit...\n\n";
  std::cout << "/////////////////////////////////////////////////////////\n\n";

  std::cout << "Give me a space-separated list of numbers.\n";
  std::cout << "Type [q or Q] to quit\n\n";

  std::string str;
  while (getline(std::cin, str)) {
    if (str.empty() || str[0] == 'q' || str[0] == 'Q')
      break;

    std::vector<double> v;
    if (Shell::parseNumbers(str.begin(), str.end(), v)) {
      std::cout << "-------------------------\n";
      std::cout << "Parsing succeeded\n";
      std::cout << str << " Parses OK: " << std::endl;

      for (std::vector<double>::size_type i = 0; i < v.size(); ++i)
        std::cout << i << ": " << v[i] << std::endl;

      std::cout << "\n-------------------------\n";
    } else {
      std::cout << "-------------------------\n";
      std::cout << "Parsing failed\n";
      std::cout << "-------------------------\n";
    }
  }

  std::cout << "Bye... :-) \n\n";
  return;
}

void
debugFS()
{
  Filesystem::Filesystem fs;

  fs.printRecursive();
}
