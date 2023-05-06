

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
#include "request_processor.h"
#include "response.h"

boost::asio::ip::tcp::socket& session::socket() { return socket_; }

void session::start() {
  socket_.async_read_some(
      boost::asio::buffer(data_, max_length),
      boost::bind(&session::handle_read, this, boost::asio::placeholders::error,
                  boost::asio::placeholders::bytes_transferred));
}
void session::handle_read(const boost::system::error_code& error,
                          size_t bytes_transferred) {
  boost::system::error_code ec;
  boost::asio::ip::tcp::endpoint endpoint = socket_.remote_endpoint(ec);
  std::string client_ip;
  if (ec == boost::system::errc::success) {
    client_ip = endpoint.address().to_string();
  } else {
    client_ip = "Unknown IP";
    LOG(warning) << "Failed to obtain remote endpoint of socket: "
                 << format_error(ec);
  }

  if (error == boost::system::errc::success) {
    RequestProcessor req_processor;
    Response res(&socket_);  // Pass the socket to the response object
    req_processor.RouteRequest(data_, serving_config_, res, client_ip);
    if (!res.has_written_http_response()) {
      res.write_http_response();
    }
  } else {
    log_error(error, "An error occurred in handle_read");
  }
  delete this;
}
