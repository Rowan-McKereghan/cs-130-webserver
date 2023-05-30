#include "crud_handler_factory.h"

#include "crud_filesystem_manager.h"
#include "logger.h"

CrudHandlerFactory::CrudHandlerFactory(NginxConfig* config) : config_(config) {}

CrudHandler* CrudHandlerFactory::CreateHandler(std::string& file_path, const std::string& client_ip) {
  CrudFileSystemManager* manager = new CrudFileSystemManager;
  std::string complete_file_path = GetCompleteFilePath(config_, file_path);
  return new CrudHandler(complete_file_path, entity_to_id_, manager, client_ip);
}