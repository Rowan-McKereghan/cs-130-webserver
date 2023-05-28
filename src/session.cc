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
#include <thread>

#include "logger.h"
#include "request_dispatcher.h"

Session::Session(boost::asio::io_service& io_service, ServingConfig serving_config)
    : socket_(io_service), _timer(io_service), io_service_(io_service), serving_config_(serving_config) {}

boost::asio::ip::tcp::socket& Session::get_socket() { return socket_; }

void Session::timeout(const boost::system::error_code& error) {
  if (error == boost::asio::error::operation_aborted) {
    return;
  }

  LOG(info) << "Bad HTTP Request. Session Timeout.";

  RequestDispatcher req_dispatcher;
  req_dispatcher.BadRequest(res);
  timeout_check = true;
  boost::beast::http::async_write(
      socket_, res, boost::bind(&Session::HandleWrite, Session::shared_from_this(), boost::asio::placeholders::error));
  socket_.close();
}

void Session::Start() {
  // threads active at the same time within the same process are guaranteed to have the same ID, this output can only
  // be reliably used to determine if a spawned session is running on a different thread from the server
  LOG(info) << "Session in thread with ID " << std::this_thread::get_id();

  _timer.expires_from_now(boost::posix_time::seconds(1));
  _timer.async_wait(boost::bind(&Session::timeout, this, boost::asio::placeholders::error));

  // threads active at the same time within the same process are guaranteed to have the same ID, this output can only
  // be reliably used to determine if a spawned session is running on a different thread from the server
  LOG(info) << "Session in thread with ID " << std::this_thread::get_id();

  _timer.expires_from_now(boost::posix_time::seconds(1));
  _timer.async_wait(boost::bind(&Session::timeout, this, boost::asio::placeholders::error));

  req = std::make_shared<boost::beast::http::request<boost::beast::http::string_body>>();
  std::shared_ptr<Session> this_shared_ptr = Session::shared_from_this();
  boost::beast::http::async_read(socket_, request_buffer, *req,
                                 boost::bind(&Session::HandleRead, this_shared_ptr, boost::asio::placeholders::error,
                                             boost::asio::placeholders::bytes_transferred));
  io_service_.run();
}

// functions extract I/O operation to allow rest of the logic to be unit tested
void Session::HandleRead(const boost::system::error_code& error, size_t bytes_transferred) {
  if (timeout_check) {  // if we timed out, immediately exit
    return;             // necessary because of the case where a malformed request makes async_read hang
  }                     // and not execute HandleRead() until the session object is deleted in timeout()
  boost::system::error_code ec;
  boost::asio::ip::tcp::endpoint endpoint = socket_.remote_endpoint(ec);
  std::string client_ip;
  if (ec == boost::system::errc::success) {
    client_ip = endpoint.address().to_string();
  } else {
    client_ip = "Unknown IP";
    LOG(warning) << "Failed to obtain remote endpoint of socket: " << FormatError(ec);
  }

  PrepareResponse(error, bytes_transferred, client_ip);
  // TODO: async_write() was causing a segfault. Figure out why and possibly change back.
  boost::beast::http::async_write(
      socket_, res, boost::bind(&Session::HandleWrite, Session::shared_from_this(), boost::asio::placeholders::error));
  // size_t bytes_t = boost::beast::http::write(socket_, res);
  // if (bytes_t < 0) {
  //   LOG(error) << "An error occurred writing to the socket.";
  // }
}

void Session::PrepareResponse(const boost::system::error_code& error, size_t bytes_transferred, std::string client_ip) {
  RequestDispatcher req_dispatcher;
  if (error == boost::system::errc::success) {
    _timer.cancel();

    if ((*req).version() != 11) {
      LOG(trace) << "Unsupported HTTP version: HTTP " << std::to_string((*req).version() / 10) << "."
                 << std::to_string((*req).version() % 10);
    }

    req_dispatcher.RouteRequest(*req, res, serving_config_, client_ip);

    time_t t;
    struct tm* myTime;
    char date[100];
    t = time(NULL);
    myTime = gmtime(&t);
    strftime(date, sizeof(date), "Date: %a, %d %b %G %T GMT", myTime);
    res.set(boost::beast::http::field::date, std::string(date));
  } else {
    LOG(info) << "An error " << error.message() << " in HandleRead\n";
    if (std::find(std::begin(parse_errors), std::end(parse_errors), error) !=
        std::end(parse_errors)) {  // check to serve 400 on req parsing error
      req_dispatcher.BadRequest(res);
    } else {
      boost::beast::ostream(res.body()) << "500 Internal Server Error";

      res.version(11);
      res.result(INTERNAL_SERVER_ERROR);
      res.set(boost::beast::http::field::content_type, "text/HTML");
      res.prepare_payload();
    }
  }
}

void Session::HandleWrite(const boost::system::error_code& error) {
  LOG(info) << "should have written";
  if (error != boost::system::errc::success) {
    LogError(error, "An error occurred in handle_write");
  }
  socket_.close();
}