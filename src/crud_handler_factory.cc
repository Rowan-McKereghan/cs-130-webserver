#include "crud_handler_factory.h"

#include "logger.h"

CrudHandlerFactory::CrudHandlerFactory(NginxConfig* config) : config_(config) {}

CrudHandler* CrudHandlerFactory::CreateHandler(std::string& file_path) {
  std::string complete_file_path = GetCompleteFilePath(config_, file_path);
  return new CrudHandler(file_path, entity_to_id_);
}