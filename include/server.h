#ifndef SERVER_H
#define SERVER_H

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/filesystem.hpp>
#include <cstdlib>
#include <iostream>

#include "I_session.h"

// server that accepts http connections over a specified port

class Server {
 public:
  Server(boost::asio::io_service& io_service, short port,
         std::function<I_session*(boost::asio::io_service&)> session_constructor);
  virtual void StartAccept();

  virtual void HandleAccept(I_session* new_session, const boost::system::error_code& error);

  I_session* get_cur_session();

  boost::asio::io_service& io_service_;
  boost::asio::ip::tcp::acceptor acceptor_;
  std::function<I_session*(boost::asio::io_service&)> session_constructor_;
  I_session* cur_session_;
};

#endif