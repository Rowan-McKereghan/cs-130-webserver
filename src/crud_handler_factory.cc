#include "crud_handler_factory.h"

#include "logger.h"

CrudHandlerFactory::CrudHandlerFactory(NginxConfig* config) : config_(config) {}

std::string CrudHandlerFactory::GetRoot() { return ""; }

std::string CrudHandlerFactory::GetCompleteFilePath(std::string& file_path) { return ""; }

CrudHandler* CrudHandlerFactory::CreateHandler(std::string& file_path) { return new CrudHandler(file_path); }