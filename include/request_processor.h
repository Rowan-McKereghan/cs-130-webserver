#ifndef REQUEST_PROCESSOR_H
#define REQUEST_PROCESSOR_H
#include <boost/filesystem.hpp>
#include <string>

#include "request.h"
#include "request_handler_echo.h"
#include "request_handler_static.h"
#include "response.h"
class RequestProcessor {
 public:
  void RouteRequest(std::string req, boost::filesystem::path root, Response &res);
};
#endif