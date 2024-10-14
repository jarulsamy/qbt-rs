#pragma once

#include "../debug.h"
#include "../logger.h"
#include "qbt.h"
#include "torrent.h"
#include "web.h"

#include <optional>
#include <stdexcept>
#include <string>

#include <boost/json.hpp>
#include <boost/json/array.hpp>
#include <boost/json/object.hpp>
#include <boost/json/parse.hpp>
#include <boost/json/serialize.hpp>
#include <boost/json/value.hpp>
#include <boost/log/trivial.hpp>
#include <curl/curl.h>
#include <curl/easy.h>
#include <fmt/core.h>

template<typename OutputIterator>
void
Qbittorrent::getTorrentList(OutputIterator output)

{
  const std::string endpoint = buildUrl("torrents/info");
  std::string resp;
  CURLcode res = GET(curl, endpoint, &resp);
  if (res != CURLE_OK) {
    const std::string err = curl_easy_strerror(res);
    LOG_LOC(fatal) << err;
  }

  boost::json::value jvp = boost::json::parse(resp);
  boost::json::array array = jvp.as_array();
  for (auto& v : array) {
    auto obj = v.as_object();

    Torrent t(this, obj);
    *(output++) = t;
  }
}
