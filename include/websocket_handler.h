#ifndef WEBSOCKET_HANDLER_H
#define WEBSOCKET_HANDLER_H

#include <boost/asio.hpp>
#include <boost/asio/strand.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/bind.hpp>
#include <boost/filesystem.hpp>
#include <cstdlib>
#include <memory>
#include <string>
#include <vector>

#include "global_websocket_state.h"

class GlobalWebsocketState;

class WebsocketHandler : public std::enable_shared_from_this<WebsocketHandler> {
  boost::beast::flat_buffer incoming_buffer_;
  boost::beast::websocket::stream<boost::asio::ip::tcp::socket> ws_;
  std::shared_ptr<GlobalWebsocketState> state_;
  std::vector<std::shared_ptr<boost::beast::flat_buffer const>> queue_;
  boost::asio::io_service::strand strand_;
  std::string channel_;

 public:
  WebsocketHandler(boost::asio::io_service::strand& strand, boost::asio::ip::tcp::socket& socket,
                   std::shared_ptr<GlobalWebsocketState> state);
  ~WebsocketHandler();

  void Run(boost::beast::http::request<boost::beast::http::string_body> req);
  void Broadcast(std::shared_ptr<boost::beast::flat_buffer const> const& message);

  void OnBroadcast(std::shared_ptr<boost::beast::flat_buffer const> const& message);
  void LogInternalError(boost::beast::error_code ec, const std::string& message);
  void OnConnect(boost::beast::error_code ec);
  void OnRecieve(boost::beast::error_code ec, std::size_t bytes_transferred);
  void OnWrite(boost::beast::error_code ec, std::size_t bytes_transferred);
};

#endif