#ifndef REQUEST_H
#define REQUEST_H
#include <string>
#include <vector>

#include "HTTPHeader.h"
class Request {
 public:
  std::string method;
  std::string uri;
  std::string uri_base_path;
  int http_version;
  std::vector<HTTPHeader> headers;
  std::string body;
  const std::string& raw_request;
  static Request ParseHTTPRequest(const std::string& req);
  Request(const std::string& req) : raw_request(req) {}

 private:
  static std::string extract_uri_base_path(const std::string& uri);
};
#endif