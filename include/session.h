#ifndef SESSION_H
#define SESSION_H

#include <time.h>

#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/bind.hpp>
#include <boost/filesystem.hpp>
#include <cstdlib>
#include <iostream>
#include <string>

#include "I_session.h"
#include "config_parser.h"
#include "serving_config.h"

// session for handling async reads and writes through a socket

const int kMaxLength = 1024;

class Session : public I_session {
 public:
  Session(boost::asio::io_service& io_service, ServingConfig serving_config);

  boost::asio::ip::tcp::socket& get_socket() override;

  void Start() override;

  void HandleRead(const boost::system::error_code& error, size_t bytes_transferred);

  void HandleWrite(const boost::system::error_code& error);

  boost::asio::ip::tcp::socket socket_;
  char data_[kMaxLength];
  std::string HTTPResponse_;
  boost::beast::flat_buffer request_buffer;
  std::shared_ptr<boost::beast::http::request<boost::beast::http::string_body>> req;
  ServingConfig serving_config_;
};

#endif