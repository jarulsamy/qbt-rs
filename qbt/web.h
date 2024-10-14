#pragma once

#include "curl/curl.h"
#include "curl/easy.h"

#include <string>

size_t
curlStrWriteCallback(char* contents, size_t size, size_t nmemb, std::string* s);

CURLcode
POST(CURL* handle,
     const std::string url,
     const std::string& postFields,
     std::string* resp);

CURLcode
GET(CURL* handle, const std::string url, std::string* resp);
