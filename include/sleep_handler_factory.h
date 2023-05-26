#ifndef SLEEP_HANDLER_FACTORY_H
#define SLEEP_HANDLER_FACTORY_H
#include <string>

#include "I_request_handler_factory.h"
#include "config_parser.h"
#include "sleep_handler.h"

class SleepHandlerFactory : public I_RequestHandlerFactory {
 public:
  SleepHandler* CreateHandler(std::string& uri_path);
};
#endif