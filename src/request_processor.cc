#include "request_processor.h"

#include <boost/algorithm/string.hpp>
#include <iostream>
#include <sstream>
#include <string>

#include "config_parser.h"
#include "logger.h"
#include "request.h"

using namespace std;

std::string RequestProcessor::ExtractUriBasePath(const std::string& uri) {
  std::string stripped_uri = uri;

  // Strip any query string from the URI
  std::size_t query_pos = stripped_uri.find('?');
  if (query_pos != std::string::npos) {
    stripped_uri = stripped_uri.substr(0, query_pos);
  }

  // Strip any trailing slashes from the URI
  boost::algorithm::trim_right_if(stripped_uri,
                                  boost::algorithm::is_any_of("/"));

  // Add back the final slash if the URI is now empty
  if (stripped_uri.empty()) {
    stripped_uri = "/";
  }

  return stripped_uri;
}

void RequestProcessor::RouteRequest(Request& req, Response& res,
                                    ServingConfig serving_config,
                                    std::string client_ip) {
  auto uri_base_path = RequestProcessor::ExtractUriBasePath(req.uri_);
  LOG(info) << "Client with IP: " << client_ip << " accessed URI: " << req.uri_;

  // Check if the URI matches any of the echo paths
  for (const auto& echo_path : serving_config.echo_paths_) {
    if (uri_base_path == echo_path) {
      LOG(info) << "Request matched to echo path: " << echo_path;
      RequestHandlerEcho handler;
      handler.HandleRequest(req, res);
      return;
    }
  }

  // Check if the URI matches any of the file paths
  for (const auto& file_path : serving_config.static_file_paths_) {
    if (uri_base_path.find(file_path.first) == 0) {
      std::string absolute_file_path =
          file_path.second + uri_base_path.substr(file_path.first.length());
      LOG(info) << "Request matched to " << file_path.first
                << ", attempting to server file: " << absolute_file_path;
      boost::filesystem::path final_file_path(absolute_file_path);
      std::cout << boost::filesystem::absolute(absolute_file_path).string()
                << std::endl;
      RequestHandlerStatic handler(final_file_path);
      handler.HandleRequest(req, res);
      return;
    }
  }

  // If neither echo path nor file path matches, return a 400 Bad Request
  // response
  LOG(warning) << "Client with IP: " << client_ip
               << " tried to access invalid URI: " << req.uri_;

  res.set_error_response(BAD_REQUEST);
}
