
#ifndef SERVING_CONFIG_H
#define SERVING_CONFIG_H

#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "config_parser.h"

const int INVALID_PATH = 1;
const int PRIVILEGED_DIR = 2;
const int MULTIPLE_ROOTS = 3;
const int INVALID_STATIC_URI = 4;
const int INVALID_ECHO_URI = 5;

struct ServingConfig {
 public:
  // Contains the path we are serving on (e.g., "/static") and the local
  // filesystem path (e.g., "/root/dir") in a pair
  std::vector<std::pair<std::string, std::string>> static_file_paths_;

  // Contains path to perform echoing (e.g., "/echo")
  std::vector<std::string> echo_paths_;

  int port_;
  bool SetPortNumber(NginxConfig* config);
  int SetPaths(NginxConfig* config);

 private:
  int ValidatePaths();
};

#endif
