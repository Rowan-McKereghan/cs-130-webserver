#ifndef CRUD_HANDLER_FACTORY_H
#define CRUD_HANDLER_FACTORY_H
#include <list>
#include <string>
#include <unordered_map>
#include <set>

#include "I_file_handler_factory.h"
#include "config_parser.h"
#include "crud_handler.h"

class CrudHandlerFactory : public I_FileHandlerFactory {
 public:
  CrudHandlerFactory(NginxConfig* config);
  CrudHandler* CreateHandler(std::string& uri_path);

 private:
  NginxConfig* config_;
  unordered_map<std::string, std::set<int>> entity_to_id_;  // map of entity name to set of IDs
};
#endif