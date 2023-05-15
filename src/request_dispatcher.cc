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
  auto handler_factories_ = serving_config.handler_factories_;
  LOG(info) << "Client with IP: " << client_ip << " accessed URI: " << req.uri_;
  NginxConfig config;  // dummy config for now
  // Check if the URI matches any of the echo paths
  if (handler_factories_.find(uri_base_path) != handler_factories_.end()) {
    LOG(info) << "Request matched to echo path: " << uri_base_path;
    EchoHandlerFactory* factory =
        dynamic_cast<EchoHandlerFactory*>(handler_factories_[uri_base_path]);
    EchoHandler* handler = factory->CreateHandler(uri_base_path);
    handler->HandleRequest(req, res);
    delete handler;
    return;
  }

  // strip away last path level to prevent matching filename as part of
  // directory

  // url_base_path will never have trailing slashes
  std::string cur_path = uri_base_path.substr(0, uri_base_path.rfind('/'));

  // longest prefix matching by iteratively stripping away levels until match is
  // found, if one exists
  while (cur_path.rfind('/') != std::string::npos) {
    if (handler_factories_.find(cur_path) != handler_factories_.end()) {
      LOG(info) << "Request matched to " << cur_path;
      StaticHandlerFactory* factory =
          dynamic_cast<StaticHandlerFactory*>(handler_factories_[cur_path]);
      // cur_path.length() + 1 will never be out of bounds since we never
      // have trailing slashes and we will not enter the loop body in the
      // case of cur_path == "/"
      std::string file_path = uri_base_path.substr(cur_path.length() + 1);
      StaticHandler* handler = factory->CreateHandler(file_path);
      handler->HandleRequest(req, res);
      delete handler;
      return;
    }
    cur_path = cur_path.substr(0, cur_path.rfind('/'));
  }

  // If neither echo path nor file path matches, return a 400 Bad Request
  // response
  LOG(warning) << "Client with IP: " << client_ip
               << " tried to access invalid URI: " << req.uri_;

  res.set_error_response(BAD_REQUEST);
}