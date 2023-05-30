#ifndef HEALTH_HANDLER_FACTORY_H
#define HEALTH_HANDLER_FACTORY_H
#include <string>

#include "I_request_handler_factory.h"
#include "config_parser.h"
#include "health_handler.h"

class HealthHandlerFactory : public I_RequestHandlerFactory {
 public:
  HealthHandler* CreateHandler(std::string& uri_path, const std::string& client_ip);
};
#endif