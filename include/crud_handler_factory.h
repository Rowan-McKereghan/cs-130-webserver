#ifndef CRUD_HANDLER_FACTORY_H
#define CRUD_HANDLER_FACTORY_H
#include <list>
#include <string>
#include <unordered_map>

#include "I_request_handler_factory.h"
#include "config_parser.h"
#include "crud_handler.h"

class CrudHandlerFactory : public I_RequestHandlerFactory {
 public:
  CrudHandlerFactory(NginxConfig* config);
  CrudHandler* CreateHandler(std::string& uri_path);
  // These are the same as the methods from the static handler factory
  // Could maybe make them static methods in the base class and change GetRoot to take a config object
  std::string GetCompleteFilePath(std::string& file_path);
  std::string GetRoot();

 private:
  NginxConfig* config_;
  unordered_map<std::string, std::list<int>> entity_to_id_;  // map of entity name to list of IDs
};
#endif