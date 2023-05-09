#ifndef REQUEST_PROCESSOR_H
#define REQUEST_PROCESSOR_H
#include <boost/filesystem.hpp>
#include <string>

#include "config_parser.h"
#include "request.h"
#include "request_handler_echo.h"
#include "request_handler_static.h"
#include "response.h"
#include "serving_config.h"

class RequestProcessor {
 public:
  void RouteRequest(Request& req, Response& res, ServingConfig serving_config,
                     std::string client_ip);

 private:
  std::string ExtractUriBasePath(const std::string& uri);
};
#endif