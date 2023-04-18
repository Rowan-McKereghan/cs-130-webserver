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
  HTTPResponse = "HTTP/1.1 200 OK\n";
  t = time(NULL);
  myTime = gmtime(&t);
   strftime(date, sizeof(date), "Date: %a, %d %b %G %T GMT", myTime);
  std::string addDate(date);
  HTTPResponse += addDate + "\n";
  HTTPResponse += "Server: webserver\n";
  //HTTPResponse += "Content-Length: " + + "\n"; //later for when we need to deal with message bodies. optional in 1.1 but not in other versions
  HTTPResponse += "Content-Type: text/plain\n";
  HTTPResponse += "\r\n";
  //boost::system::error_code httperror;
  boost::asio::async_write(socket_, 
                    boost::asio::buffer(HTTPResponse),  
                    boost::bind(&session::handleHTTPwrite, this,
                              boost::asio::placeholders::error));
  socket_.async_read_some(
      boost::asio::buffer(data_, max_length),
      boost::bind(&session::handle_read, this, boost::asio::placeholders::error,
                  boost::asio::placeholders::bytes_transferred));
}

void session::handle_read(const boost::system::error_code& error,
                          size_t bytes_transferred) {
  if (!error) {
   if (data_[bytes_transferred - 1] == '\n' && data_[bytes_transferred - 2] == '\n') { //last is \n\n
      session::end_of_request = true;
    }
    if (data_[bytes_transferred - 1] == '\n' && data_[bytes_transferred - 2] == '\r' && data_[bytes_transferred - 3] == '\n' && data_[bytes_transferred - 4] == '\r') {
      session::end_of_request = true; //last is \r\n\r\n
    }
    boost::asio::async_write(socket_,
                             boost::asio::buffer(data_, bytes_transferred),
                             boost::bind(&session::handle_write, this,
                                         boost::asio::placeholders::error));
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

void session::handleHTTPwrite(const boost::system::error_code& error) {
    //std::cout << "made it to http" << std::endl;
    if (error) {
      delete this;
    }
}
