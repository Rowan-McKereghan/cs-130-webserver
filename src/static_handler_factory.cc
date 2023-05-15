#include "static_handler_factory.h"

#include <string>

#include "config_parser.h"
#include "logger.h"
#include "static_handler.h"

StaticHandlerFactory::StaticHandlerFactory(NginxConfig* config)
    : config_(config) {}

std::string StaticHandlerFactory::GetRoot() {
  if (config_->statements_.size() == 1) {
    std::vector<std::string> tokens = config_->statements_[0]->tokens_;
    // looking for this format:
    // root /serving_directory
    if (tokens.size() >= 2 && tokens[0] == "root") {
      std::string root = tokens[1];
      // add back trailing slash if necessary
      if (root.empty() || root[root.size() - 1] != '/') {
        root += '/';
      }
      return root;
    }
  }
  return "";
}

std::string StaticHandlerFactory::GetCompleteFilePath(std::string& file_path) {
  std::string root = GetRoot();
  return root + file_path;
}

StaticHandler* StaticHandlerFactory::CreateHandler(std::string& file_path) {
  std::string complete_file_path = GetCompleteFilePath(file_path);
  LOG(info) << "Attempting to serve file: " << complete_file_path;
  return new StaticHandler(complete_file_path);
}