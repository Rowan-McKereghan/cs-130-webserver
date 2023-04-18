#ifndef SESSION_H
#define SESSION_H

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <cstdlib>
#include <iostream>
#include <string>
#include <time.h>

// session for handling async reads and writes through a socket

class session {
 public:
  session(boost::asio::io_service& io_service);
  boost::asio::ip::tcp::socket& socket();

  void start();

 private:
  void handle_read(const boost::system::error_code& error,
                   size_t bytes_transferred);

  void handle_write(const boost::system::error_code& error);

  void handleHTTPwrite(const boost::system::error_code& error);


  boost::asio::ip::tcp::socket socket_;
  enum { max_length = 1024 };
  //TODO: include something like "char full_request[8192]", the general max length for http requests with bodies for webservers is 8KB
  char data_[max_length];
  char date[100];
  time_t t;
  struct tm *myTime;
  bool end_of_request = false;
  std::string HTTPResponse; 

};

#endif