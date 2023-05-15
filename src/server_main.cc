//
// async_tcp_echo_server.cpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2017 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/filesystem.hpp>
#include <cstdlib>
#include <iostream>
#include <string>

#include "config_parser.h"
#include "logger.h"
#include "server.h"
#include "serving_config.h"
#include "session.h"

int main(int argc, char* argv[]) {
  try {
    // Log SIGINT
    signal(SIGINT, [](int signum) {
      LOG(info) << "Received SIGINT: " << signum << ", Exiting";
      std::exit(signum);
    });

    // Configure Boost logging
    InitLogging();

    LOG(info) << "Starting server...";

    if (argc != 2) {
      LOG(error) << "Usage: webserver <config_file_path>";
      return 1;
    }

    boost::asio::io_service io_service;

    NginxConfigParser parser;
    NginxConfig config;
    ServingConfig serving_config;

    if (!parser.Parse(argv[1], &config)) {
      LOG(fatal) << "Failed to parse config file";
      return 1;
    }

    if (!serving_config.SetPortNumber(&config)) {
      LOG(fatal) << "Failed to get port number";
      return 1;
    } else {
      LOG(info) << "Config file parsed successfully. Port: "
                << serving_config.port_;
    }

    if (serving_config.SetPaths(&config)) {
      LOG(fatal)
          << "The config file contains not valid static or echoing paths";
      return 1;
    }

    // Capture serving_config by value to avoid having to store it in the server
    // obj
    auto session_constructor =
        [serving_config](boost::asio::io_service& io_service) {
          return new Session(io_service, serving_config);
        };

    Server s(io_service, serving_config.port_, session_constructor);
    s.StartAccept();
    io_service.run();
  } catch (std::exception& e) {
    LOG(fatal) << "Encountered exception: " << e.what() << ", Exiting";
  }

  return 0;
}
