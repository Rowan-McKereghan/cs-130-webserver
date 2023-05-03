#include "server.h"

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/filesystem.hpp>
#include <cstdlib>
#include <iostream>

#include "I_session.h"
#include "logger.h"

using boost::asio::ip::tcp;

server::server(
    boost::asio::io_service& io_service, short port, boost::filesystem::path root,
    std::function<I_session*(boost::asio::io_service&, boost::filesystem::path)> session_constructor)
    : io_service_(io_service),
      acceptor_(io_service, tcp::endpoint(tcp::v4(), port)), 
      root_(root),
      session_constructor_(session_constructor) {}

void server::start_accept() {
  cur_session = session_constructor_(io_service_, root_);
  acceptor_.async_accept(cur_session->socket(),
                         boost::bind(&server::handle_accept, this, cur_session,
                                     boost::asio::placeholders::error));
}

void server::handle_accept(I_session* new_session,
                           const boost::system::error_code& ec) {
  if (ec == boost::system::errc::success) {
    LOG(trace) << "Handling Request.";
    new_session->start();
  } else {
    log_error(ec, "Failed to handle request");
  }

  LOG(trace) << "Finished handling request, accepting new requests.";
  start_accept();
}

I_session* server::get_cur_session() { return cur_session; }