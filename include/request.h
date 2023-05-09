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
  std::string method_;
  std::string uri_;
  boost::filesystem::path file_path_;
  int http_version_;
  std::vector<HTTPHeader> headers_;
  std::string body_;
  const std::string& raw_request_;
};
#endif