#include "echo_handler_factory.h"

#include "echo_handler.h"

EchoHandler* EchoHandlerFactory::CreateHandler(std::string& uri, NginxConfig& config) {
  return new EchoHandler();
}