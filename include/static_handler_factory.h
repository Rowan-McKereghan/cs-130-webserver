#ifndef STATIC_HANDLER_FACTORY_H
#define STATIC_HANDLER_FACTORY_H
#include <string>

#include "I_request_handler_factory.h"
#include "config_parser.h"
#include "static_handler.h"

class StaticHandlerFactory : public I_RequestHandlerFactory {
 public:
  StaticHandlerFactory(NginxConfig* config);
  StaticHandler* CreateHandler(std::string& file_path);
  std::string GetCompleteFilePath(std::string& file_path);
  std::string GetRoot();
  
 private:
  NginxConfig* config_;
};
#endif