#ifndef REQUEST_HANDLER_FACTORY_H
#define REQUEST_HANDLER_FACTORY_H

#include <iostream>
#include <string>

#include "I_request_handler.h"
#include "config_parser.h"

using namespace std;

class I_RequestHandlerFactory {
 public:
  virtual I_RequestHandler* CreateHandler(std::string& uri_path, const std::string& client_ip) = 0;
};

#endif