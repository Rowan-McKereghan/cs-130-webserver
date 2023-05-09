#ifndef REQUEST_H
#define REQUEST_H
#include <boost/filesystem.hpp>
#include <string>
#include <vector>

#include "HTTPHeader.h"
class Request {
 public:
  // TODO: add support for request with body
  Request(const std::string& req_str);
  std::string method;
  std::string uri;
  boost::filesystem::path file_path_;
  int http_version;
  std::vector<HTTPHeader> headers;
  std::string body;
  const std::string& raw_request;
};
#endif