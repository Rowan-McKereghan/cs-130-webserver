#include "request_processor.h"

#include <iostream>
#include <sstream>

#include "logger.h"
#include "request.h"
std::string Request::extract_uri_base_path(const std::string &uri) {
  if (uri.empty() || uri[0] != '/') {
    LOG(warning) << "URI is invalid: " << uri;
    return "";
  }
  size_t pos = uri.find('/', 1);
  if (pos == std::string::npos) {
    pos = uri.length();
  }
  return uri.substr(1, pos - 1);
}
Request Request::ParseHTTPRequest(const std::string &req) {
  Request req_obj(req);
  std::istringstream request_stream(req);
  std::string line;
  // Parse the first line (Request-Line)
  if (std::getline(request_stream, line)) {
    std::istringstream line_stream(line);
    line_stream >> req_obj.method >> req_obj.uri;
    std::string http_version_string;
    line_stream >> http_version_string;
    if (http_version_string == "HTTP/1.1") {
      req_obj.http_version = 1;
    } else {
      // TODO: Handle other HTTP versions or default to 1.0
      LOG(trace) << "Unsupported HTTP version string: " << http_version_string;
      req_obj.http_version = 0;
    }
  }
  // Parse the header fields
  while (std::getline(request_stream, line) && !line.empty() && line != "\r") {
    HTTPHeader header;
    std::size_t colon_pos = line.find(':');
    if (colon_pos != std::string::npos) {
      header.name = line.substr(0, colon_pos);
      header.value = line.substr(colon_pos + 1);
    }
    req_obj.headers.push_back(header);
  }
  req_obj.uri_base_path = Request::extract_uri_base_path(req_obj.uri);
  return req_obj;
}
void RequestProcessor::ParseRequest(string req, Response &res) {
  Request req_obj = Request::ParseHTTPRequest(req);
  LOG(trace) << "URI is: " << req_obj.uri
             << " with basepath: " << req_obj.uri_base_path;
  if (req_obj.uri_base_path == "echo") {
    RequestHandlerEcho handler;
    handler.HandleRequest(req_obj, res);
  } else if (req_obj.uri_base_path == "static") {
    // TODO - get base_dir from config parser
    string base_dir = "/src";
    RequestHandlerStatic handler(base_dir);
    handler.HandleRequest(req_obj, res);
  } else {
    LOG(warning) << "Invalid URI Acessed: " << req_obj.uri;
  }
}