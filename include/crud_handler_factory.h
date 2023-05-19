#ifndef CRUD_HANDLER_FACTORY_H
#define CRUD_HANDLER_FACTORY_H
#include <string>

#include "I_request_handler_factory.h"
#include "config_parser.h"
#include "crud_handler.h"

class CrudHandlerFactory : public I_RequestHandlerFactory {
 public:
  CrudHandlerFactory(NginxConfig* config);
  CrudHandler* CreateHandler(std::string& uri_path);
  std::string GetCompleteFilePath(std::string& file_path);
  std::string GetRoot();

 private:
  NginxConfig* config_;
};
#endif