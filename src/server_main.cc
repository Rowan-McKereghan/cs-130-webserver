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
#include "server.h"
#include "session.h"

int main(int argc, char* argv[]) {
  try {
    if (argc != 2) {
      fprintf(stderr, "Usage: async_tcp_echo_server <config_file_path>\n");
      return 1;
    }

    boost::asio::io_service io_service;

    NginxConfigParser parser;
    NginxConfig config;
    short port;
    if (!parser.Parse(argv[1], &config) ||
        !parser.GetPortNumber(&config, &port))
      return 1;

    server s(io_service, port);

    io_service.run();
  } catch (std::exception& e) {
    fprintf(stderr, "Exception: %s\n", e.what());
  }

  return 0;
}
