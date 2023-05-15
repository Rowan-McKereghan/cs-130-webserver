#ifndef ECHO_HANDLER_FACTORY_H
#define ECHO_HANDLER_FACTORY_H
#include <string>

#include "I_request_handler_factory.h"
#include "config_parser.h"
#include "echo_handler.h"

class EchoHandlerFactory : public I_RequestHandlerFactory {
 public:
  // NginxConfig argument is intended, just has't been implemented yet
  EchoHandler* CreateHandler(std::string& uri, NginxConfig& config);
};
#endif