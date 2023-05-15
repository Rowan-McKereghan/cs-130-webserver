#ifndef ECHO_HANDLER_H
#define ECHO_HANDLER_H

#include "I_request_handler.h"
#include "request.h"
#include "response.h"

class EchoHandler : public I_RequestHandler {
 public:
  // No constructor needed since it is config invariant
  void HandleRequest(const Request &req, Response &res) override;
};

#endif