#ifndef STATIC_HANDLER_FACTORY_H
#define STATIC_HANDLER_FACTORY_H
#include <string>

#include "config_parser.h"
#include "file_handler_factory.h"
#include "static_handler.h"

class StaticHandlerFactory : public FileHandlerFactory {
 public:
  // possible TODO: move config parsing to handler constructor
  StaticHandlerFactory(NginxConfig* config);
  StaticHandler* CreateHandler(std::string& uri_path);

 private:
  NginxConfig* config_;
};
#endif