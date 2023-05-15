#ifndef REQUEST_HANDLER_FACTORY_H
#define REQUEST_HANDLER_FACTORY_H

#include <iostream>
#include <string>

#include "I_request_handler.h"
#include "config_parser.h"
#include "request.h"
#include "response.h"

using namespace std;

class I_RequestHandlerFactory {
 public:
  // NginxConfig argument is intended, just has't been implemented yet
  virtual I_RequestHandler* CreateHandler(std::string& uri,
                                          NginxConfig& config) = 0;
};

#endif