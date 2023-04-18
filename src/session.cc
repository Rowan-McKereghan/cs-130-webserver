#include "session.h"

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <cstdlib>
#include <iostream>
#include <string>
#include <time.h>


session::session(boost::asio::io_service& io_service) : socket_(io_service) {}

boost::asio::ip::tcp::socket& session::socket() { return socket_; }

void session::start() {
  socket_.async_read_some(
      boost::asio::buffer(data_, max_length),
      boost::bind(&session::handle_read, this, boost::asio::placeholders::error,
                  boost::asio::placeholders::bytes_transferred));
}

void session::handle_read(const boost::system::error_code& error,
                          size_t bytes_transferred) {
  if (!error) {
    t = time(NULL);
    myTime = gmtime(&t);
    strftime(date, sizeof(date), "Date: %a, %d %b %G %T GMT", myTime);
    std::string addDate(date);
    HTTPResponse += addDate + "\n";
    HTTPResponse += "Server: webserver\n";
    HTTPResponse += "Content-Length: " + std::to_string(bytes_transferred) + "\n";
    HTTPResponse += "Content-Type: text/plain\n";
    HTTPResponse += "\r\n";
    boost::asio::async_write(socket_,
                             boost::asio::buffer(HTTPResponse, HTTPResponse.length()),
                             boost::bind(&session::handle_write, this,
                                         boost::asio::placeholders::error));
    boost::asio::async_write(socket_,
                             boost::asio::buffer(data_, bytes_transferred),
                             boost::bind(&session::handle_write, this,
                                         boost::asio::placeholders::error));
    HTTPResponse = "HTTP/1.1 200 OK\n";
  } else {
    delete this;
  }
}

void session::handle_write(const boost::system::error_code& error) {
  if (!error) {
    socket_.async_read_some(
        boost::asio::buffer(data_, max_length),
        boost::bind(&session::handle_read, this,
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred));
  } else {
    delete this;
  }
}