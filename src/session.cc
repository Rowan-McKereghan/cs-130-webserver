

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

using namespace std;

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
    const char* response_c_string = res.generate_http_response().c_str();
    boost::asio::async_write(
        socket_,
        boost::asio::buffer(response_c_string,
                            res.generate_http_response().length()),
        boost::bind(&session::handle_write, this,
                    boost::asio::placeholders::error));
  } else {
    log_error(error, "An error occurred in handle_read");
    delete this;
  }
}
void session::handle_write(const boost::system::error_code& error) {
  if (error != boost::system::errc::success) {
    log_error(error, "An error occurred in handle_write");
  }

  delete this;
}