#ifndef WEBSOCKET_INFO_HANDLER_H
#define WEBSOCKET_INFO_HANDLER_H

#include <boost/beast/http.hpp>

#include "I_request_handler.h"
#include "global_websocket_state.h"

class WebsocketInfoHandler : public I_RequestHandler {
 public:
  WebsocketInfoHandler(std::shared_ptr<GlobalWebsocketState> state);
  StatusCode HandleRequest(const boost::beast::http::request<boost::beast::http::string_body> req,
                           boost::beast::http::response<boost::beast::http::dynamic_body>& res);

 private:
  std::shared_ptr<GlobalWebsocketState> state_;
};

#endif