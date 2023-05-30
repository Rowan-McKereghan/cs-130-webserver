#ifndef ECHO_HANDLER_FACTORY_H
#define ECHO_HANDLER_FACTORY_H
#include <string>

#include "I_request_handler_factory.h"
#include "config_parser.h"
#include "echo_handler.h"

class EchoHandlerFactory : public I_RequestHandlerFactory {
 public:
  EchoHandler* CreateHandler(std::string& uri_path, const std::string& client_ip);
};
#endif