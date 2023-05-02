#ifndef SESSION_H
#define SESSION_H

#include <time.h>

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <cstdlib>
#include <iostream>
#include <string>

#include "I_session.h"

// session for handling async reads and writes through a socket

class session : public I_session {
 public:
  session(boost::asio::io_service& io_service);
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
};

#endif