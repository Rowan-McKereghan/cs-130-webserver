#ifndef STATIC_HANDLER_FACTORY_H
#define STATIC_HANDLER_FACTORY_H
#include <string>

#include "I_file_handler_factory.h"
#include "config_parser.h"
#include "static_handler.h"

class StaticHandlerFactory : public I_FileHandlerFactory {
 public:
  // possible TODO: move config parsing to handler constructor
  StaticHandlerFactory(NginxConfig* config);
  StaticHandler* CreateHandler(std::string& uri_path);

 private:
  NginxConfig* config_;
};
#endif