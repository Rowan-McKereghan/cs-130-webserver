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
#include "global_websocket_state.h"
#include "serving_config.h"
// session for handling async reads and writes through a socket

const int kMaxLength = 1024;

// inheritance from std::enable_shared_from_this is required to obtain a valid shared pointer referring to the "this"
// pointer
class Session : public I_session, public std::enable_shared_from_this<Session> {
 public:
  Session(boost::asio::io_service& io_service, ServingConfig serving_config,
          std::shared_ptr<GlobalWebsocketState> state);

  virtual boost::asio::ip::tcp::socket& get_socket() override;

  virtual void Start() override;

  virtual void HandleRead(const boost::system::error_code& error, size_t bytes_transferred);

  virtual void PrepareResponse(const boost::system::error_code& error, size_t bytes_transferred, std::string client_ip);

  virtual void HandleWrite(const boost::system::error_code& error);

  virtual void Timeout(const boost::system::error_code& error);

  boost::asio::io_service& io_service_;
  boost::asio::ip::tcp::socket socket_;
  boost::asio::io_service::strand strand_;
  char data_[kMaxLength];
  std::string HTTPResponse_;
  boost::beast::flat_buffer request_buffer;
  std::shared_ptr<boost::beast::http::request<boost::beast::http::string_body>> req;
  boost::beast::http::response<boost::beast::http::dynamic_body> res;
  ServingConfig serving_config_;
  boost::asio::deadline_timer _timer;
  bool timeout_check = false;
  boost::beast::http::error parse_errors[6] = {
      boost::beast::http::error::bad_target,      boost::beast::http::error::bad_method,
      boost::beast::http::error::bad_field,       boost::beast::http::error::bad_value,
      boost::beast::http::error::bad_line_ending, boost::beast::http::error::bad_version};

  std::shared_ptr<GlobalWebsocketState> state_;
};

#endif