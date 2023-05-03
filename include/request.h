#ifndef REQUEST_H
#define REQUEST_H
#include <boost/filesystem.hpp>
#include <string>
#include <vector>

#include "HTTPHeader.h"
class Request {
 public:
  std::string method;
  std::string uri;
  boost::filesystem::path file_path_;
  int http_version;
  std::vector<HTTPHeader> headers;
  std::string body;
  const std::string& raw_request;
  static Request ParseHTTPRequest(const std::string& req);
  Request(const std::string& req) : raw_request(req) {}
};
#endif