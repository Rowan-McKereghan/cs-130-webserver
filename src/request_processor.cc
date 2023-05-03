#include "request_processor.h"

#include <boost/algorithm/string.hpp>
#include <iostream>
#include <sstream>
#include <string>

#include "config_parser.h"
#include "logger.h"
#include "request.h"

using namespace std;

std::string RequestProcessor::extract_uri_base_path(const std::string &uri) {
  std::string stripped_uri = uri;

  // Strip any query string from the URI
  std::size_t query_pos = stripped_uri.find('?');
  if (query_pos != std::string::npos) {
    stripped_uri = stripped_uri.substr(0, query_pos);
  }

  // Strip any trailing slashes from the URI
  boost::algorithm::trim_right_if(stripped_uri,
                                  boost::algorithm::is_any_of("/"));

  // Add back the final slash if the URI is empty or just "/"
  if (stripped_uri.empty() || stripped_uri == "/") {
    stripped_uri = "/";
  }

  return stripped_uri;
}

void RequestProcessor::RouteRequest(string req, ServingConfig serving_config,
                                    Response &res, std::string client_ip) {
  Request req_obj = Request::ParseHTTPRequest(req);
  auto uri_base_path = RequestProcessor::extract_uri_base_path(req_obj.uri);
  LOG(info) << "Client with IP: " << client_ip
            << " accessed URI: " << req_obj.uri;

  // Check if the URI matches any of the echo paths
  for (const auto &echo_path : serving_config.echo_paths) {
    if (uri_base_path == echo_path) {
      LOG(info) << "Request matched to echo path: " << echo_path;
      RequestHandlerEcho handler;
      handler.HandleRequest(req_obj, res);
      return;
    }
  }

  // Check if the URI matches any of the file paths
  for (const auto &file_path : serving_config.static_file_paths) {
    if (uri_base_path.find(file_path.first) == 0) {
      LOG(info) << "Request matched to " << file_path.first
                << " serving files from: " << file_path.second;
      std::string absolute_file_path =
          file_path.second + uri_base_path.substr(file_path.first.length());
      LOG(info) << "Request matched to file" << absolute_file_path;
      boost::filesystem::path final_file_path(absolute_file_path);
      RequestHandlerStatic handler(final_file_path);
      handler.HandleRequest(req_obj, res);
      return;
    }
  }

  // If neither echo path nor file path matches, return a 404 Not Found
  // response
  LOG(warning) << "Client with IP: " << client_ip
               << " tried to access invalid URI: " << req_obj.uri;
  HTTPHeader contentType;
  contentType.name = "Content-Type";
  contentType.value = "text/HTML";
  HTTPHeader contentLength;
  contentLength.name = "Content-Length";
  contentLength.value = std::to_string(res.data.length());
  vector<HTTPHeader> headers;
  headers.push_back(contentType);
  headers.push_back(contentLength);
  res.headers = headers;
  res.status_code = BAD_REQUEST;
}
