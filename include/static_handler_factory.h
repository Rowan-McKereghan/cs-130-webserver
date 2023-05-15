#ifndef STATIC_HANDLER_FACTORY_H
#define STATIC_HANDLER_FACTORY_H
#include <string>

#include "I_request_handler_factory.h"
#include "config_parser.h"
#include "static_handler.h"

class StaticHandlerFactory : public I_RequestHandlerFactory {
 public:
  // NginxConfig argument is intended, just has't been implemented yet
  StaticHandler* CreateHandler(std::string& uri, NginxConfig& config);
};
#endif