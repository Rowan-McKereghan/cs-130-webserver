#include "session.h"

#include <time.h>

#include <boost/asio.hpp>
#include <boost/beast/http.hpp>
#include <boost/bind.hpp>
#include <boost/filesystem.hpp>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iostream>
#include <string>

#include "logger.h"
#include "request_dispatcher.h"

Session::Session(boost::asio::io_service& io_service, ServingConfig serving_config)
    : socket_(io_service), _timer(io_service), serving_config_(serving_config) {}

boost::asio::ip::tcp::socket& Session::get_socket() { return socket_; }

void Session::timeout(const boost::system::error_code& error) {
  if (error == boost::asio::error::operation_aborted) {
    return;
  }

  LOG(info) << "Bad HTTP Request. Session Timeout.";

  boost::beast::http::response<boost::beast::http::dynamic_body> res;

  boost::beast::ostream(res.body()) << "400 Bad Request";

  res.version(11);
  res.result(BAD_REQUEST);
  res.set(boost::beast::http::field::content_type, "text/HTML");
  res.prepare_payload();

  size_t bytes_t = boost::beast::http::write(socket_, res);
  if (bytes_t < 0) {
    LOG(error) << "An error occurred writing to the socket.";
  }

  timeout_check = true;
  delete this;
}

void Session::Start() {
  _timer.expires_from_now(boost::posix_time::seconds(1));
  _timer.async_wait(boost::bind(&Session::timeout, this, boost::asio::placeholders::error));

  req = std::make_shared<boost::beast::http::request<boost::beast::http::string_body>>();
  boost::beast::http::async_read(socket_, request_buffer, *req,
                                 boost::bind(&Session::HandleRead, this, boost::asio::placeholders::error,
                                             boost::asio::placeholders::bytes_transferred));
}

void Session::HandleRead(const boost::system::error_code& error, size_t bytes_transferred) {
  boost::system::error_code ec;
  boost::asio::ip::tcp::endpoint endpoint = socket_.remote_endpoint(ec);
  std::string client_ip;
  if (timeout_check == true) {  // fatal crash if we try to obtain socket after timing out.
    return;
  }
  if (ec == boost::system::errc::success) {
    client_ip = endpoint.address().to_string();
  } else {
    client_ip = "Unknown IP";
    LOG(warning) << "Failed to obtain remote endpoint of socket: " << FormatError(ec);
  }

  boost::beast::http::response<boost::beast::http::dynamic_body> res;
  if (error == boost::system::errc::success) {
    _timer.cancel();

    if ((*req).version() != 11) {
      LOG(trace) << "Unsupported HTTP version: HTTP " << std::to_string((*req).version() / 10) << "."
                 << std::to_string((*req).version() % 10);
    }
    RequestDispatcher req_dispatcher;

    req_dispatcher.RouteRequest(*req, res, serving_config_, client_ip);

    time_t t;
    struct tm* myTime;
    char date[100];
    t = time(NULL);
    myTime = gmtime(&t);
    strftime(date, sizeof(date), "Date: %a, %d %b %G %T GMT", myTime);
    res.set(boost::beast::http::field::date, std::string(date));

    // TODO: async_write() was causing a segfault. Figure out why and possibly change back.
    size_t bytes_t = boost::beast::http::write(socket_, res);
    if (bytes_t < 0) {
      LOG(error) << "An error occurred writing to the socket.";
    }
  } else {
    LOG(info) << "An error " << error.message() << " in HandleRead\n";
    if (error == boost::beast::http::error::bad_target ||  // parsing errors, not read I/O errors, serve 400
        error == boost::beast::http::error::bad_method || error == boost::beast::http::error::bad_field ||
        error == boost::beast::http::error::bad_value || error == boost::beast::http::error::bad_line_ending ||
        error == boost::beast::http::error::bad_version) {
      boost::beast::ostream(res.body()) << "400 Bad Request";

      res.version(11);
      res.result(BAD_REQUEST);
      res.set(boost::beast::http::field::content_type, "text/HTML");
      res.prepare_payload();

      size_t bytes_t = boost::beast::http::write(socket_, res);
      if (bytes_t < 0) {
        LOG(error) << "An error occurred writing to the socket.";
      }
    } else {
      boost::beast::ostream(res.body()) << "500 Internal Server Error";

      res.version(11);
      res.result(INTERNAL_SERVER_ERROR);
      res.set(boost::beast::http::field::content_type, "text/HTML");
      res.prepare_payload();

      size_t bytes_t = boost::beast::http::write(socket_, res);
      if (bytes_t < 0) {
        LOG(error) << "An error occurred writing to the socket.";
      }
    }
  }
  delete this;
}

void Session::HandleWrite(const boost::system::error_code& error) {
  if (error != boost::system::errc::success) {
    LogError(error, "An error occurred in handle_write");
    delete this;
  }
}