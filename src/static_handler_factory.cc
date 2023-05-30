#include "static_handler_factory.h"

#include <string>

#include "config_parser.h"
#include "logger.h"
#include "static_handler.h"

StaticHandlerFactory::StaticHandlerFactory(NginxConfig* config) : config_(config) {}

StaticHandler* StaticHandlerFactory::CreateHandler(std::string& file_path, const std::string& client_ip) {
  std::string complete_file_path = GetCompleteFilePath(config_, file_path);
  LOG(info) << "Attempting to serve file: " << complete_file_path;
  return new StaticHandler(complete_file_path, client_ip);
}