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
#include <cstdlib>
#include <iostream>

#include "config_parser.h"
#include "logger.h"
#include "server.h"
#include "session.h"

int main(int argc, char* argv[]) {
  try {
    init_logging();
    LOG(info) << "Starting server...";

    if (argc != 2) {
      LOG(error) << "Usage: async_tcp_echo_server <config_file_path>";
      return 1;
    }

    boost::asio::io_service io_service;

    NginxConfigParser parser;
    NginxConfig config;
    short port;
    if (!parser.Parse(argv[1], &config)) {
      LOG(fatal) << "Failed to parse config file";
      return 1;
    } else if (!parser.GetPortNumber(&config, &port)) {
      LOG(fatal) << "Failed to get port number";
      return 1;
    }

    LOG(info) << "Config file parsed successfully. Port: " << port;

    auto session_constructor = [](boost::asio::io_service& io_service) {
      return new session(io_service);
    };

    server s(io_service, port, session_constructor);
    s.start_accept();
    io_service.run();
  } catch (std::exception& e) {
    LOG(fatal) << "Exception: " << e.what();
  }

  return 0;
}
