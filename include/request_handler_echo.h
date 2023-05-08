#ifndef REQUEST_HANDLER_ECHO_H
#define REQUEST_HANDLER_ECHO_H

#include "I_request_handler.h"
#include "request.h"
#include "response.h"

class RequestHandlerEcho : public I_RequestHandler {
 public:
  // No constructor needed since it is config invariant
  void HandleRequest(const Request &req, Response &res) override;
};

#endif