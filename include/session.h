#ifndef SESSION_H
#define SESSION_H

#include <time.h>

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/filesystem.hpp>
#include <cstdlib>
#include <iostream>
#include <string>

#include "I_session.h"
#include "config_parser.h"

// session for handling async reads and writes through a socket

class session : public I_session {
 public:
  session(boost::asio::io_service& io_service, ServingConfig serving_config)
      : socket_(io_service), serving_config_(serving_config) {}

  boost::asio::ip::tcp::socket& socket() override;

  void start() override;

  void handle_read(const boost::system::error_code& error,
                   size_t bytes_transferred);

  void handle_write(const boost::system::error_code& error);

  void handle_http_write(const boost::system::error_code& error);

  boost::asio::ip::tcp::socket socket_;
  enum { max_length = 1024 };
  char data_[max_length];
  char date[100];
  time_t t;
  struct tm* myTime;
  std::string HTTPResponse;
  ServingConfig serving_config_;
};

#endif