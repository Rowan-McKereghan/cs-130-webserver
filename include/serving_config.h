#ifndef SERVING_CONFIG_H
#define SERVING_CONFIG_H

#include <I_request_handler_factory.h>

#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include "global_websocket_state.h"
#include <vector>

#include "config_parser.h"

const int MULTIPLE_USES = 1;
const int INVALID_URI = 2;

struct ServingConfig {
 public:
  std::unordered_map<string, I_RequestHandlerFactory*> handler_factories_;
  std::unordered_map<string, string> path_to_handler_name_;

  int port_;
  bool SetPortNumber(NginxConfig* config);
  int SetPaths(NginxConfig* config, std::shared_ptr<GlobalWebsocketState>);

 private:
  int ValidatePaths();
};

#endif
