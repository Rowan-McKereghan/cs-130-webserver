

#include "session.h"

#include <time.h>

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/filesystem.hpp>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>

#include "logger.h"
#include "request.h"
#include "request_processor.h"
#include "response.h"

Session::Session(boost::asio::io_service& io_service, ServingConfig serving_config)
      : socket_(io_service), serving_config_(serving_config) {}

boost::asio::ip::tcp::socket& Session::get_socket() { return socket_; }

void Session::Start() {
  socket_.async_read_some(
      boost::asio::buffer(data_, max_length),
      boost::bind(&Session::HandleRead, this, boost::asio::placeholders::error,
                  boost::asio::placeholders::bytes_transferred));
}
void Session::HandleRead(const boost::system::error_code& error,
                          size_t bytes_transferred) {
  boost::system::error_code ec;
  boost::asio::ip::tcp::endpoint endpoint = socket_.remote_endpoint(ec);
  std::string client_ip;
  if (ec == boost::system::errc::success) {
    client_ip = endpoint.address().to_string();
  } else {
    client_ip = "Unknown IP";
    LOG(warning) << "Failed to obtain remote endpoint of socket: "
                 << FormatError(ec);
  }

  if (error == boost::system::errc::success) {
    RequestProcessor req_processor;
    Request req(data_);
    Response res(&socket_);  // Pass the socket to the response object
    req_processor.RouteRequest(req, res, serving_config_, client_ip);
    if (!res.get_wrote_http_response()) {
      res.WriteHTTPResponse();
    }
  } else {
    LogError(error, "An error occurred in handle_read");
  }
  delete this;
}
