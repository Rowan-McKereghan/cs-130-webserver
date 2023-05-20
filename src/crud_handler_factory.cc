#include "crud_handler_factory.h"

#include "logger.h"

CrudHandlerFactory::CrudHandlerFactory(NginxConfig* config) : config_(config) {}

std::string CrudHandlerFactory::GetRoot() {
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

std::string CrudHandlerFactory::GetCompleteFilePath(std::string& file_path) {
  std::string root = GetRoot();
  return root + file_path;
}

CrudHandler* CrudHandlerFactory::CreateHandler(std::string& file_path) {
  return new CrudHandler(file_path, entity_to_id_);
}