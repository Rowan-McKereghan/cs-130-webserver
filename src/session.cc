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
    : socket_(io_service), serving_config_(serving_config) {}

boost::asio::ip::tcp::socket& Session::get_socket() { return socket_; }

void Session::Start() {
  req = std::make_shared<boost::beast::http::request<boost::beast::http::string_body>>();
  std::shared_ptr<Session> this_shared_ptr = Session::shared_from_this();
  boost::beast::http::async_read(socket_, request_buffer, *req,
                                 boost::bind(&Session::HandleRead, this_shared_ptr, boost::asio::placeholders::error,
                                             boost::asio::placeholders::bytes_transferred));
}

// functions extract I/O operation to allow rest of the logic to be unit tested
void Session::HandleRead(const boost::system::error_code& error, size_t bytes_transferred) {
  boost::system::error_code ec;
  boost::asio::ip::tcp::endpoint endpoint = socket_.remote_endpoint(ec);
  std::string client_ip;
  if (ec == boost::system::errc::success) {
    client_ip = endpoint.address().to_string();
  } else {
    client_ip = "Unknown IP";
    LOG(warning) << "Failed to obtain remote endpoint of socket: " << FormatError(ec);
  }

  if (this->PrepareResponse(error, bytes_transferred, client_ip)) {
    // TODO: async_write() was causing a segfault. Figure out why and possibly change back.
    size_t bytes_t = boost::beast::http::write(socket_, res);
    if (bytes_t < 0) {
      LOG(error) << "An error occurred writing to the socket.";
    }
  }
}

bool Session::PrepareResponse(const boost::system::error_code& error, size_t bytes_transferred, std::string client_ip) {
  if (error == boost::system::errc::success) {
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

    return true;
  } else {
    LOG(info) << "An error occurred in HandleRead\n";
    if (error == boost::beast::http::error::bad_target) {
      boost::beast::ostream(res.body()) << "400 Bad Request";

      res.version(11);
      res.result(BAD_REQUEST);
      res.set(boost::beast::http::field::content_type, "text/HTML");
      res.prepare_payload();

      return true;
    } else {
      LOG(error) << "The error in HandleRead was a read I/O error\n";
      return false;
    }
  }
}

void Session::HandleWrite(const boost::system::error_code& error) {
  if (error != boost::system::errc::success) {
    LogError(error, "An error occurred in handle_write");
  }
}