#include "websocket_handler.h"

#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/bind.hpp>
#include <boost/filesystem.hpp>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "global_websocket_state.h"
#include "logger.h"

WebsocketHandler::WebsocketHandler(boost::asio::io_service::strand& strand, boost::asio::ip::tcp::socket& socket,
                                   std::shared_ptr<GlobalWebsocketState> state)
    : ws_(std::move(socket)), state_(state), strand_(strand) {}

WebsocketHandler::~WebsocketHandler() { state_->Remove(this->channel_, this); }

void WebsocketHandler::Run(boost::beast::http::request<boost::beast::http::string_body> req) {
  ws_.set_option(boost::beast::websocket::stream_base::timeout{
      std::chrono::seconds(5),   // handshake_timeout
      std::chrono::minutes(15),  // idle_timeout
      true                       // keep_alive_pings
  });
  // need to send messages as binary to be compatible with Protobufs, otherwise we have utf-8 encoding errors
  ws_.binary(true);
  ws_.set_option(boost::beast::websocket::stream_base::decorator(
      [](boost::beast::websocket::response_type& res) { res.set(boost::beast::http::field::server, "Webserver"); }));

  this->channel_ = req.target().to_string();

  ws_.async_accept(req, [self = shared_from_this()](boost::system::error_code ec) { self->OnConnect(ec); });
}

void WebsocketHandler::LogInternalError(boost::beast::error_code ec, const std::string& message) {
  // Normal operation
  if (ec == boost::beast::net::error::operation_aborted || ec == boost::beast::websocket::error::closed) return;

  LogError(ec, message);
}

void WebsocketHandler::OnConnect(boost::beast::error_code ec) {
  if (ec != boost::system::errc::success) {
    LogError(ec, "OnConnect");
    return;
  }

  LOG(info) << "Connect Handshake from IP: " << ws_.next_layer().remote_endpoint().address().to_string()
            << " Channel: " << this->channel_;
            // << " On Thread ID: " << std::this_thread::get_id() << " Channel: " << this->channel_;

  state_->Add(this->channel_, this);
  ws_.async_read(incoming_buffer_,
                 [self = shared_from_this()](boost::system::error_code ec, std::size_t bytes_transferred) {
                   self->OnRecieve(ec, bytes_transferred);
                 });
}

void WebsocketHandler::OnRecieve(boost::beast::error_code ec, std::size_t) {
  if (ec != boost::system::errc::success) {
    LogError(ec, "OnRecieve");
    return;
  }

  state_->Broadcast(this->channel_, incoming_buffer_);
  incoming_buffer_.consume(incoming_buffer_.size());
  ws_.async_read(incoming_buffer_,
                 [self = shared_from_this()](boost::system::error_code ec, std::size_t bytes_transferred) {
                   self->OnRecieve(ec, bytes_transferred);
                 });
}

void WebsocketHandler::Broadcast(std::shared_ptr<boost::beast::flat_buffer const> const& message) {
  LOG(info) << "Broadcasting message from IP: " << ws_.next_layer().remote_endpoint().address().to_string()
            << " Channel: " << this->channel_;
            // << " On Thread ID: " << std::this_thread::get_id << " Channel: " << this->channel_;
  // Post to strand to prevent concurrent access
  boost::asio::post(strand_, [self = shared_from_this(), message]() { self->OnBroadcast(message); });
}

void WebsocketHandler::OnBroadcast(std::shared_ptr<boost::beast::flat_buffer const> const& message) {
  // Check strand requirement, it won't perform the operation if this condition is false
  if (!strand_.running_in_this_thread()) {
    // If we're not in the correct strand, post this operation back to the correct strand
    return boost::asio::post(strand_, [self = shared_from_this(), message]() { self->OnBroadcast(message); });
  }

  queue_.push_back(message);

  if (queue_.size() > 1) return;

  // Write immediately
  ws_.async_write(boost::asio::buffer(queue_.front()->cdata()),
                  [self = shared_from_this()](boost::system::error_code ec, std::size_t bytes_transferred) {
                    self->OnWrite(ec, bytes_transferred);
                  });
}

void WebsocketHandler::OnWrite(boost::beast::error_code ec, std::size_t) {
  if (ec != boost::system::errc::success) {
    LogError(ec, "OnWrite");
    return;
  }

  queue_.erase(queue_.begin());

  if (!queue_.empty()) {
    ws_.async_write(boost::asio::buffer(queue_.front()->cdata()),
                    [self = shared_from_this()](boost::system::error_code ec, std::size_t bytes_transferred) {
                      self->OnWrite(ec, bytes_transferred);
                    });
  }
}
