#ifndef REQUEST_DISPATCHER_H
#define REQUEST_DISPATCHER_H
#include <boost/filesystem.hpp>
#include <string>

#include "config_parser.h"
#include "echo_handler.h"
#include "request.h"
#include "response.h"
#include "serving_config.h"
#include "static_handler.h"

class RequestDispatcher {
 public:
  void RouteRequest(Request& req, Response& res, ServingConfig serving_config,
                    std::string client_ip);

 private:
  std::string ExtractUriBasePath(const std::string& uri);
};
#endif