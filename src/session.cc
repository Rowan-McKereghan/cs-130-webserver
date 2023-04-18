#include "session.h"

#include <time.h>

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <cstdlib>
#include <iostream>
#include <string>

session::session(boost::asio::io_service& io_service) : socket_(io_service) {}

boost::asio::ip::tcp::socket& session::socket() { return socket_; }

void session::format_http_response() {
  HTTPResponse = "HTTP/1.1 200 OK\n";
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

void session::check_if_http_request_ends(size_t bytes_transferred) {
  if (data_[bytes_transferred - 1] == '\n' && data_[bytes_transferred - 2] == '\n') {  
    // check for last line of http request ("\n\n"), exit in handle_write if so
    session::end_of_request = true;
  }
  if (data_[bytes_transferred - 1] == '\n' &&
      data_[bytes_transferred - 2] == '\r' &&
      data_[bytes_transferred - 3] == '\n' &&
      data_[bytes_transferred - 4] == '\r') {
        // check for last line of http request ("\r\n\r\n", other possible case), exit in handle_write if so
        session::end_of_request = true;
  }
}

void session::start() {
  session::format_http_response();
  boost::asio::async_write(socket_, boost::asio::buffer(HTTPResponse),
                           boost::bind(&session::handle_http_write, this,
                                       boost::asio::placeholders::error));
  socket_.async_read_some(
      boost::asio::buffer(data_, max_length),
      boost::bind(&session::handle_read, this, boost::asio::placeholders::error,
                  boost::asio::placeholders::bytes_transferred));
}

void session::handle_read(const boost::system::error_code& error,
                          size_t bytes_transferred) {
  if (!error) {
    session::check_if_http_request_ends(bytes_transferred);
    boost::asio::async_write(socket_,
                             boost::asio::buffer(data_, bytes_transferred),
                             boost::bind(&session::handle_write, this,
                                         boost::asio::placeholders::error));
  } else {
    delete this;
  }
}

void session::handle_write(const boost::system::error_code& error) {
  if (!error && !session::end_of_request) {
    socket_.async_read_some(
        boost::asio::buffer(data_, max_length),
        boost::bind(&session::handle_read, this,
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred));
  } else {
    delete this;
  }
}

void session::handle_http_write(const boost::system::error_code& error) {
  if (error) {
    delete this;
  }
}
