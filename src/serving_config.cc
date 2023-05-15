

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
#include "logger.h"
#include "privileged_dirs.h"

bool IsOnlyDigits(const std::string& str) {
  return std::all_of(str.begin(), str.end(),
                     [](char c) { return std::isdigit(c); });
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
            if (block_tokens.size() >= 2 && block_tokens[0] == "listen" &&
                IsOnlyDigits(block_tokens[1])) {
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

// surprising not a library function for this in boost::filesystem
// please replace if someone can find one
static bool IsParentDir(const boost::filesystem::path& parent_path,
                        const boost::filesystem::path& child_path) {
  int p_depth = std::distance(parent_path.begin(), parent_path.end());
  int c_depth = std::distance(child_path.begin(), child_path.end());

  // return false when parent has more directory levels than child
  if (p_depth > c_depth) {
    return false;
  }

  auto parent_end = parent_path.end();
  auto child_end = child_path.end();

  // loop to compare parent and child paths at each level
  for (auto parent_it = parent_path.begin(), child_it = child_path.begin();
       parent_it != parent_end; ++parent_it, ++child_it) {
    // found a part of the parent path that doesn't match child path
    // before exhausting all of parent path, so return false
    if (*parent_it != *child_it) {
      return false;
    }
  }

  // exhausted parent path, so return true
  return true;
}

static bool IsInPrivilegedDirectory(const std::string& file_path) {
  boost::filesystem::path path(file_path);
  path = boost::filesystem::absolute(path);

  // privileged_dirs is set in privileged_dirs.h
  for (const auto& privileged_dir : privileged_dirs) {
    boost::filesystem::path privileged_path(privileged_dir);
    privileged_path = boost::filesystem::absolute(privileged_path);

    // both path and privileged_path will be absolute to allow for
    // straighforward comparison without factoring in . and ..
    if (IsParentDir(privileged_path, path)) {
      // the requested file is located in a privileged directory
      return true;
    }
  }

  // the requested file is not located in a privileged directory
  return false;
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
              NginxConfig* paths_block = block_statement->child_block_.get();
              if (paths_block != nullptr) {
                if (handler_name == "StaticHandler") {
                  auto statements = paths_block->statements_;
                  std::vector<std::string> file_path_tokens =
                      statements[0]->tokens_;

                  // Remove trailing slash if it exists
                  for (auto& path : file_path_tokens) {
                    if (!path.empty() && path[path.size() - 1] == '/') {
                      path.erase(path.size() - 1);
                    }
                  }

                  if (file_path_tokens.size() >= 2 &&
                      file_path_tokens[0] == "root") {
                    static_file_paths_.push_back(
                        {serving_path, file_path_tokens[1]});
                  }
                } else if (handler_name == "EchoHandler") {
                  echo_paths_.push_back(serving_path);
                }
              }
            }
          }
        }
      }
    }
  }

  // sort files in descending order of most '/', then ascending order
  // alphabetically, allows for longest prefix matching
  std::sort(static_file_paths_.begin(), static_file_paths_.end(),
            [](std::pair<std::string, std::string> p1,
               std::pair<std::string, std::string> p2) {
              int c1 = std::count(p1.first.begin(), p1.first.end(), '/');
              int c2 = std::count(p2.first.begin(), p2.first.end(), '/');
              if (c1 != c2) {
                return c1 > c2;
              } else {
                return p1.first < p2.first;
              }
            });

  return ValidatePaths();
}

int ServingConfig::ValidatePaths() {
  // Verify file paths and remove non-existing static file paths
  for (auto itr = static_file_paths_.begin(); itr != static_file_paths_.end();
       itr++) {
    const auto& file_path = *itr;
    std::string file_path_key = file_path.first;
    std::string file_path_value = file_path.second;
    if (IsValidURI(file_path_key)) {
      if (!boost::filesystem::exists(file_path_value)) {
        LOG(error) << file_path_value
                   << " is an invalid file path with serving URI: "
                   << file_path_key << ". The current working directory is: "
                   << boost::filesystem::current_path();
        return INVALID_PATH;
      } else if (IsInPrivilegedDirectory(file_path_value)) {
        LOG(error) << file_path_value
                   << " is an in a privileged directory. The current working "
                      "directory is: "
                   << boost::filesystem::current_path();
        return PRIVILEGED_DIR;
      } else if (itr != static_file_paths_.begin() &&
                 std::prev(itr)->first == file_path_key) {
        LOG(error) << "Multiple root directories specified for "
                   << file_path_value << ". The current working directory is: "
                   << boost::filesystem::current_path();
        return MULTIPLE_ROOTS;
      }
    } else {
      LOG(error) << file_path_key
                 << "is an invalid URI. The current working directory is: "
                 << boost::filesystem::current_path();
      return INVALID_STATIC_URI;
    }
  }

  // Verify echo paths
  for (auto echo_path : echo_paths_) {
    if (!IsValidURI(echo_path)) {
      LOG(error) << echo_path << " is an invalid echoing path";
      return INVALID_ECHO_URI;
    }
  }

  return 0;
}
