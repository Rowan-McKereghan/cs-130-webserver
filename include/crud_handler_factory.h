#ifndef CRUD_HANDLER_FACTORY_H
#define CRUD_HANDLER_FACTORY_H
#include <list>
#include <string>
#include <unordered_map>

#include "config_parser.h"
#include "crud_handler.h"
#include "file_handler_factory.h"

class CrudHandlerFactory : public FileHandlerFactory {
 public:
  CrudHandlerFactory(NginxConfig* config);
  CrudHandler* CreateHandler(std::string& uri_path);

 private:
  NginxConfig* config_;
  unordered_map<std::string, std::list<int>> entity_to_id_;  // map of entity name to list of IDs
};
#endif