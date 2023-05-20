#include "I_file_handler_factory.h"

#include <string>

#include "config_parser.h"

std::string I_FileHandlerFactory::GetRoot(NginxConfig* config) {
  if (config->statements_.size() == 1) {
    std::vector<std::string> tokens = config->statements_[0]->tokens_;
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

std::string I_FileHandlerFactory::GetCompleteFilePath(NginxConfig* config, std::string& file_path) {
  std::string root = GetRoot(config);
  return root + file_path;
}
