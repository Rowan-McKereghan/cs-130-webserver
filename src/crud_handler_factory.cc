#include "crud_handler_factory.h"

#include "crud_filesystem_manager.h"
#include "logger.h"

CrudHandlerFactory::CrudHandlerFactory(NginxConfig* config) : config_(config) {}

CrudHandler* CrudHandlerFactory::CreateHandler(std::string& file_path) {
  CrudFileSystemManager* manager = new CrudFileSystemManager;
  std::string complete_file_path = GetCompleteFilePath(config_, file_path);
  std::string entity = "shoes";
 
  return new CrudHandler(complete_file_path, entity_to_id_, manager);
}