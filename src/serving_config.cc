

#include "serving_config.h"

#include <algorithm>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/regex.hpp>
#include <cctype>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <memory>
#include <stack>
#include <string>
#include <vector>

#include "config_parser.h"
#include "crud_handler_factory.h"
#include "echo_handler_factory.h"
#include "health_handler_factory.h"
#include "logger.h"
#include "not_found_handler_factory.h"
#include "privileged_dirs.h"
#include "sleep_handler_factory.h"
#include "static_handler_factory.h"

bool IsOnlyDigits(const std::string& str) {
  return std::all_of(str.begin(), str.end(), [](char c) { return std::isdigit(c); });
}

bool ServingConfig::SetPortNumber(NginxConfig* config) {
  if (config != nullptr) {  // handle empty config
    /* need to look for this structure:
    server {
      ...
      listen <port #>
      ...
    }
    */
    for (auto statement : config->statements_) {
      std::vector<std::string> tokens = statement->tokens_;
      // find outer "server" declaration
      if (tokens.size() >= 1 && tokens[0] == "server") {
        NginxConfig* child_block = statement->child_block_.get();
        if (child_block != nullptr) {
          for (auto block_statement : child_block->statements_) {
            std::vector<std::string> block_tokens = block_statement->tokens_;
            // look for listen <port #> in child block
            if (block_tokens.size() >= 2 && block_tokens[0] == "listen" && IsOnlyDigits(block_tokens[1])) {
              port_ = stoi(block_tokens[1]);
              return true;
            }
          }
        }
      }
    }
  }
  return false;
}

static bool IsValidURI(const std::string& uri) {
  static const boost::regex uri_regex("^\\/[a-zA-Z0-9\\/\\._-]+$");
  return boost::regex_match(uri, uri_regex);
}

int ServingConfig::SetPaths(NginxConfig* config) {
  if (config != nullptr) {  // handle empty config
    /* need to look for this structure:
    server {
      ...
      location /echo EchoHandler {

      }
      ...
      location /static StaticHandler {
        root ./files;
      }
      ...
    }
    */
    for (auto statement : config->statements_) {
      std::vector<std::string> tokens = statement->tokens_;
      // find outer "server" declaration
      if (tokens.size() >= 1 && tokens[0] == "server") {
        NginxConfig* child_block = statement->child_block_.get();
        if (child_block != nullptr) {
          for (auto block_statement : child_block->statements_) {
            std::vector<std::string> block_tokens = block_statement->tokens_;
            // look for static_file_paths or echo_paths block in child block
            if (block_tokens.size() >= 3 && block_tokens[0] == "location") {
              std::string serving_path = block_tokens[1];
              std::string handler_name = block_tokens[2];
              // unfortunately, cannot check for this error in postprocessing
              // since a duplicate value is simple overwritten in a
              // unordered_map
              if (handler_factories_.find(serving_path) != handler_factories_.end()) {
                LOG(fatal) << "Multiple uses specified for endpoint " << serving_path
                           << ". The current working directory is: " << boost::filesystem::current_path();
                return MULTIPLE_USES;
              }
              NginxConfig* paths_block = block_statement->child_block_.get();
              if (paths_block != nullptr) {
                path_to_handler_name_[serving_path] = handler_name;
                if (handler_name == "StaticHandler") {
                  handler_factories_[serving_path] = new StaticHandlerFactory(paths_block);
                } else if (handler_name == "EchoHandler") {
                  handler_factories_[serving_path] = new EchoHandlerFactory();
                } else if (handler_name == "CrudHandler") {
                  handler_factories_[serving_path] = new CrudHandlerFactory(paths_block);
                } else if (handler_name == "HealthHandler") {
                  handler_factories_[serving_path] = new HealthHandlerFactory();
                } else if (handler_name == "TestHandler") {
                  // add other test endpoints as subdirectories when necessary
                  handler_factories_[serving_path + "/sleep"] = new SleepHandlerFactory();
                }
              }
            }
          }
        }
      }
    }
  }

  auto status = ValidatePaths();

  path_to_handler_name_["/"] = "NotFoundHandler";
  handler_factories_["/"] = new NotFoundHandlerFactory();

  return status;
}

int ServingConfig::ValidatePaths() {
  // Verify file paths and remove non-existing static file paths
  for (auto pair : handler_factories_) {
    std::string file_path = pair.first;
    if (!IsValidURI(file_path)) {
      LOG(fatal) << file_path
                 << " is an invalid URI. The current working directory is: " << boost::filesystem::current_path();
      return INVALID_URI;
    }
  }

  return 0;
}
