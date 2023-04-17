#ifndef SERVER_H
#define SERVER_H

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <cstdlib>
#include <iostream>

#include "session.h"

// server that accepts http connections over a specified port

class server {
 public:
  server(boost::asio::io_service& io_service, short port);

 private:
  void start_accept();

  void handle_accept(session* new_session,
                     const boost::system::error_code& error);

  boost::asio::io_service& io_service_;
  boost::asio::ip::tcp::acceptor acceptor_;
};

#endif