#ifndef SERVER_H
#define SERVER_H

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/filesystem.hpp>
#include <cstdlib>
#include <iostream>

#include "I_session.h"
#include "global_websocket_state.h"

// server that accepts http connections over a specified port

class Server {
 public:
  Server(boost::asio::io_service& io_service, short port,
         std::function<std::shared_ptr<I_session>(boost::asio::io_service&)> session_constructor);
  virtual void StartAccept();

  virtual void HandleAccept(std::shared_ptr<I_session> new_session, const boost::system::error_code& error);

  // this function's sole purpose is to serve as a wrapper for the session's Start() function to allow gmock to properly
  // track its invocations, avoiding complications from mocking a callback
  virtual void StartSession(std::shared_ptr<I_session> new_session);

  std::shared_ptr<I_session> get_cur_session();

  boost::asio::io_service& io_service_;
  boost::asio::ip::tcp::acceptor acceptor_;
  std::function<std::shared_ptr<I_session>(boost::asio::io_service&)> session_constructor_;
  std::shared_ptr<I_session> cur_session_;
  std::shared_ptr<GlobalWebsocketState> state_;
};

#endif