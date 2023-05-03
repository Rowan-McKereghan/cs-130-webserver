

#include "session.h"

#include <time.h>

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/filesystem.hpp>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>

#include "request_processor.h"
#include "response.h"

using namespace std;
#include "logger.h"
session::session(boost::asio::io_service& io_service, boost::filesystem::path root) : socket_(io_service), root_(root) {}
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
  if (ec == boost::system::errc::success) {
    LOG(trace) << "Request from IP: " << endpoint.address().to_string();
  } else {
    LOG(warning) << "Failed to obtain remote endpoint of socket: "
                 << format_error(ec);
  }
  if (error == boost::system::errc::success) {
    RequestProcessor req_processor;
    Response res(&socket_); // Pass the socket to the response object
    req_processor.RouteRequest(data_, root_, res);
    const char* response_c_string = res.generate_http_response().c_str();
    boost::asio::async_write(socket_, boost::asio::buffer(response_c_string, res.generate_http_response().length()),
                              boost::bind(&session::handle_write, this, boost::asio::placeholders::error));
  }
}
void session::handle_write(const boost::system::error_code& error) {
  if (!error) {
    delete this;
  }
}
void session::handle_http_write(const boost::system::error_code& error) {
  if (error) {
    delete this;
  }
}