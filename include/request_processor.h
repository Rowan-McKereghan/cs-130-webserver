#ifndef REQUEST_PROCESSOR_H
#define REQUEST_PROCESSOR_H

#include <string>

#include "request.h"
#include "request_handler_echo.h"
#include "request_handler_static.h"
#include "response.h"

class RequestProcessor {
 public:
  void ParseRequest(std::string req, Response &res);
};

#endif