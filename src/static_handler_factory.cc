#include "static_handler_factory.h"

#include "static_handler.h"

StaticHandler* StaticHandlerFactory::CreateHandler(std::string& uri,
                                                   NginxConfig& config) {
  return new StaticHandler(uri);
}