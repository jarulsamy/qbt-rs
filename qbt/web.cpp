#include "web.h"

#include <curl/curl.h>
#include <curl/easy.h>
#include <string>

size_t
curlStrWriteCallback(char* contents, size_t size, size_t nmemb, std::string* s)
{
  size_t newLength = size * nmemb;
  s->append(contents, newLength);
  return newLength;
}

CURLcode
POST(CURL* handle,
     const std::string url,
     const std::string& postFields,
     std::string* resp)
{
  curl_easy_setopt(handle, CURLOPT_POST, 1L);
  curl_easy_setopt(handle, CURLOPT_WRITEDATA, resp);
  curl_easy_setopt(handle, CURLOPT_URL, url.c_str());
  curl_easy_setopt(handle, CURLOPT_POSTFIELDS, postFields.c_str());

  return curl_easy_perform(handle);
}

CURLcode
GET(CURL* handle, const std::string url, std::string* resp)
{
  curl_easy_setopt(handle, CURLOPT_HTTPGET, 1L);
  curl_easy_setopt(handle, CURLOPT_WRITEDATA, resp);
  curl_easy_setopt(handle, CURLOPT_URL, url.c_str());

  return curl_easy_perform(handle);
}
