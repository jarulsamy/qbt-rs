#pragma once

#include "../debug.h"
#include "../logger.h"
#include "qbt.h"
#include "torrent.h"
#include "web.h"

#include <boost/log/trivial.hpp>
#include <curl/curl.h>
#include <curl/easy.h>

template<typename OutputIterator>
void
Torrent::getContents(OutputIterator output)
{
  const std::string myUrl = fmt::format("torrents/files?hash={}", hash);
  const std::string endpoint = parent->buildUrl(myUrl);
  std::string resp;
  CURLcode res = GET(parent->curl, endpoint, &resp);
  if (res != CURLE_OK) {
    const std::string err = curl_easy_strerror(res);
    LOG_LOC(fatal) << err;
  }

  boost::json::value jvp = boost::json::parse(resp);
  boost::json::array array = jvp.as_array();
  for (auto& v : array) {
    auto obj = v.as_object();

    Torrent::ContentItem i(obj);
    *(output++) = i;
  }
}