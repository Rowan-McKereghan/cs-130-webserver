
#include "session.h"

#include <time.h>

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <cstdlib>
#include <iostream>
#include <string>

#include "request_processor.h"
#include "response.h"
using namespace std;
#include "logger.h"
session::session(boost::asio::io_service& io_service) : socket_(io_service) {}
boost::asio::ip::tcp::socket& session::socket() { return socket_; }
void session::format_http_response() {
  HTTPResponse = "HTTP/1.1 200 OK\r\n";
  t = time(NULL);
  myTime = gmtime(&t);
  strftime(date, sizeof(date), "Date: %a, %d %b %G %T GMT", myTime);
  std::string addDate(date);
  HTTPResponse += addDate + "\n";
  HTTPResponse += "Server: webserver\n";
  // TODO: Add a field: "Content-Length: " + body length in bytes+ "\n"; to
  // response header when we need to deal with message bodies. optional in 1.1
  // but not in other versions
  HTTPResponse += "Content-Type: text/plain\n";
  HTTPResponse += "\r\n";
}

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
    Response res;
    req_processor.RouteRequest(data_, res);
    auto response = res.generate_http_response();
    response += res.data;
    const char* response_c_string = response.c_str();
    boost::asio::async_write(
        socket_, boost::asio::buffer(response_c_string, response.length()),
        boost::bind(&session::handle_write, this,
                    boost::asio::placeholders::error));
  } else {
    log_error(error, "An error occurred");
    delete this;
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