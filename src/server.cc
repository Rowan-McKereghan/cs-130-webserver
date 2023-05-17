#include "server.h"

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/filesystem.hpp>
#include <cstdlib>
#include <iostream>

#include "I_session.h"
#include "logger.h"

using boost::asio::ip::tcp;

Server::Server(boost::asio::io_service& io_service, short port,
               std::function<I_session*(boost::asio::io_service&)> session_constructor)
    : io_service_(io_service),
      acceptor_(io_service, tcp::endpoint(tcp::v4(), port)),
      session_constructor_(session_constructor) {}

void Server::StartAccept() {
  cur_session_ = session_constructor_(io_service_);
  acceptor_.async_accept(cur_session_->get_socket(),
                         boost::bind(&Server::HandleAccept, this, cur_session_, boost::asio::placeholders::error));
}

void Server::HandleAccept(I_session* new_session, const boost::system::error_code& ec) {
  if (ec == boost::system::errc::success) {
    LOG(trace) << "Handling Request.";
    new_session->Start();
  } else {
    LogError(ec, "Failed to handle request");
  }

  LOG(trace) << "Finished handling request, accepting new requests.";
  StartAccept();
}

I_session* Server::get_cur_session() { return cur_session_; }