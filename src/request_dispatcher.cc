#include "request_dispatcher.h"

#include <boost/algorithm/string.hpp>
#include <iostream>
#include <sstream>
#include <string>

#include "config_parser.h"
#include "echo_handler.h"
#include "echo_handler_factory.h"
#include "logger.h"
#include "request.h"
#include "static_handler.h"
#include "static_handler_factory.h"

using namespace std;

I_RequestHandlerFactory* CreateHandlerFactory(const string& name) {
  if (name == "static") return new StaticHandlerFactory();
  if (name == "echo") return new EchoHandlerFactory();
}

std::string RequestDispatcher::ExtractUriBasePath(const std::string& uri) {
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

void RequestDispatcher::RouteRequest(Request& req, Response& res,
                                     ServingConfig serving_config,
                                     std::string client_ip) {
  std::string uri_base_path = RequestDispatcher::ExtractUriBasePath(req.uri_);
  LOG(info) << "Client with IP: " << client_ip << " accessed URI: " << req.uri_;
  NginxConfig config;  // dummy config for now
  // Check if the URI matches any of the echo paths
  for (const auto& echo_path : serving_config.echo_paths_) {
    if (uri_base_path == echo_path) {
      LOG(info) << "Request matched to echo path: " << echo_path;
      EchoHandlerFactory* handler_factory = dynamic_cast<EchoHandlerFactory*>(CreateHandlerFactory("echo"));
      EchoHandler* handler = handler_factory->CreateHandler(
          uri_base_path, config);  // TODO: actually pass in config
      handler->HandleRequest(req, res);
      delete handler_factory;
      delete handler;
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
      StaticHandlerFactory* handler_factory = dynamic_cast<StaticHandlerFactory*>(CreateHandlerFactory("static"));
      StaticHandler* handler = handler_factory->CreateHandler(
          absolute_file_path, config);  // TODO: actually pass in config
      handler->HandleRequest(req, res);
      delete handler_factory;
      delete handler;
      return;
    }
  }

  // If neither echo path nor file path matches, return a 400 Bad Request
  // response
  LOG(warning) << "Client with IP: " << client_ip
               << " tried to access invalid URI: " << req.uri_;

  res.set_error_response(BAD_REQUEST);
}