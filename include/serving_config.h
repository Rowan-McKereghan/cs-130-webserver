
#ifndef SERVING_CONFIG_H
#define SERVING_CONFIG_H

#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "config_parser.h"

struct ServingConfig {
 public:
  // Contains the path we are serving on (e.g., "/static") and the local
  // filesystem path (e.g., "/root/dir") in a pair
  std::vector<std::pair<std::string, std::string>> static_file_paths;

  // Contains path to perform echoing (e.g., "/echo")
  std::vector<std::string> echo_paths;
  
  int port;
  bool SetPortNumber(NginxConfig* config);
  bool SetPaths(NginxConfig* config);

 private:
  bool IsValidURI(const std::string& uri);
};

#endif
