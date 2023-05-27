#include "request_dispatcher.h"

#include <boost/algorithm/string.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <iostream>
#include <sstream>
#include <string>

#include "config_parser.h"
#include "echo_handler.h"
#include "echo_handler_factory.h"
#include "logger.h"
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
  boost::algorithm::trim_right_if(stripped_uri, boost::algorithm::is_any_of("/"));

  // Add back the final slash if the URI is now empty
  if (stripped_uri.empty()) {
    stripped_uri = "/";
  }

  return stripped_uri;
}

void RequestDispatcher::RouteRequest(boost::beast::http::request<boost::beast::http::string_body> req,
                                     boost::beast::http::response<boost::beast::http::dynamic_body>& res,
                                     ServingConfig serving_config, std::string client_ip) {
  bool isBad = false;
  boost::beast::http::verb methods[4] = {boost::beast::http::verb::get, boost::beast::http::verb::put,
                                         boost::beast::http::verb::post, boost::beast::http::verb::delete_};
  if (std::find(std::begin(methods), std::end(methods), req.method())) {  // check for unsupported verbs
    LOG(info) << "Unsupported HTTP verb: " << req.method_string().to_string();
    isBad = true;
  }
  if (req.version() != 11) {  // check for HTTP/1.1
    LOG(info) << "Unsupported HTTP version: HTTP " << std::to_string(req.version() / 10) << "."
              << std::to_string(req.version() % 10);
    isBad = true;
  }
  if (isBad) {  // catch bad requests early if able
    boost::beast::ostream(res.body()) << "400 Bad Request";
    res.version(req.version());
    res.result(BAD_REQUEST);
    res.set(boost::beast::http::field::content_type, "text/HTML");
    res.prepare_payload();
    return;
  }
  std::string req_uri_ = req.target().to_string();
  std::string uri_base_path = RequestDispatcher::ExtractUriBasePath(req_uri_);
  auto handler_factories_ = serving_config.handler_factories_;
  LOG(info) << "Client with IP: " << client_ip << " accessed URI: " << req_uri_;
  NginxConfig config;  // dummy config for now
  // Check if the URI matches any of the echo paths
  if (handler_factories_.find(uri_base_path) != handler_factories_.end()) {
    LOG(info) << "Request matched to path: " << uri_base_path;
    I_RequestHandler* handler = handler_factories_[uri_base_path]->CreateHandler(uri_base_path);
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
      I_RequestHandlerFactory* factory = handler_factories_[cur_path];
      // cur_path.length() + 1 will never be out of bounds since we never
      // have trailing slashes and we will not enter the loop body in the
      // case of cur_path == "/"
      std::string file_path = uri_base_path.substr(cur_path.length() + 1);
      I_RequestHandler* handler = factory->CreateHandler(file_path);
      handler->HandleRequest(req, res);
      delete handler;
      return;
    }
    cur_path = cur_path.substr(0, cur_path.rfind('/'));
  }

  // If neither echo path nor file path matches, return a 400 Bad Request
  // response
  LOG(warning) << "Client with IP: " << client_ip << " tried to access invalid URI: " << req_uri_;
  // set res to 400 bad request
  boost::beast::ostream(res.body()) << "400 Bad Request";

  res.version(req.version());
  res.result(BAD_REQUEST);
  res.set(boost::beast::http::field::content_type, "text/HTML");
  res.prepare_payload();
}