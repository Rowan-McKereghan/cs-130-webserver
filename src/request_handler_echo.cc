#include "request_handler_echo.h"

#include "logger.h"
#include "request.h"
#include "response.h"

void RequestHandlerEcho::HandleRequest(const Request &req, Response &res) {
  res.set_status_code(OK);
  res.set_body(req.raw_request);
  res.add_header("Content-Type", "text/plain");
  res.add_header("Content-Length", std::to_string(req.raw_request.length()));
}