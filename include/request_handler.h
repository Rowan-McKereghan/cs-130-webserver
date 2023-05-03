#ifndef REQUEST_HANDLER_H
#define REQUEST_HANDLER_H

#include <iostream>
#include <string>

#include "request.h"
#include "response.h"

using namespace std;

class I_RequestHandler {
 public:
  virtual void HandleRequest(const Request &req, Response &res) = 0;
};

#endif