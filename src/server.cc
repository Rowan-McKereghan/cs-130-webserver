#include "server.h"

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/filesystem.hpp>
#include <cstdlib>
#include <iostream>
#include <thread>

#include "I_session.h"
#include "logger.h"

using boost::asio::ip::tcp;

Server::Server(boost::asio::io_service& io_service, short port,
               std::function<std::shared_ptr<I_session>(boost::asio::io_service&)> session_constructor)
    : io_service_(io_service),
      acceptor_(io_service, tcp::endpoint(tcp::v4(), port)),
      session_constructor_(session_constructor) {}

void Server::StartAccept() {
  cur_session_ = session_constructor_(io_service_);
  acceptor_.async_accept(cur_session_->get_socket(),
                         boost::bind(&Server::HandleAccept, this, cur_session_, boost::asio::placeholders::error));
}

void Server::HandleAccept(std::shared_ptr<I_session> new_session, const boost::system::error_code& ec) {
  if (ec == boost::system::errc::success) {
    LOG(trace) << "Handling Request.";

    // threads active at the same time within the same process are guaranteed to have the same ID, this output can only
    // be reliably used to determine if a spawned session is running on a different thread from the server
    LOG(info) << "Server in thread with ID " << std::this_thread::get_id();
    std::thread thread(boost::bind(&I_session::Start, new_session));
    // detaching thread because we don't need to join it later
    thread.detach();
  } else {
    LogError(ec, "Failed to handle request");
  }

  LOG(trace) << "Finished handling request, accepting new requests.";
  StartAccept();
}

std::shared_ptr<I_session> Server::get_cur_session() { return cur_session_; }