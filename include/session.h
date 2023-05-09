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
#include "serving_config.h"

// session for handling async reads and writes through a socket

class Session : public I_session {
 public:
  Session(boost::asio::io_service& io_service, ServingConfig serving_config);

  boost::asio::ip::tcp::socket& get_socket() override;

  void Start() override;

  void HandleRead(const boost::system::error_code& error,
                   size_t bytes_transferred);


  boost::asio::ip::tcp::socket socket_;
  enum { max_length = 1024 };
  char data_[max_length];
  char date_[100];
  time_t t_;
  struct tm* myTime_;
  std::string HTTPResponse_;
  ServingConfig serving_config_;
};

#endif