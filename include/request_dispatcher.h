#ifndef REQUEST_DISPATCHER_H
#define REQUEST_DISPATCHER_H
#include <boost/beast/http.hpp>
#include <boost/filesystem.hpp>
#include <string>

#include "config_parser.h"
#include "echo_handler.h"
#include "serving_config.h"
#include "static_handler.h"

class RequestDispatcher {
 public:
  void RouteRequest(boost::beast::http::request<boost::beast::http::string_body> req,
                    boost::beast::http::response<boost::beast::http::dynamic_body>& res, ServingConfig serving_config,
                    std::string client_ip);

 private:
  std::string ExtractUriBasePath(const std::string& uri);
};
#endif