#include "websocket_info_handler.h"
#include "logger.h"

WebsocketInfoHandler::WebsocketInfoHandler(std::shared_ptr<GlobalWebsocketState> state) : state_(state) {}

StatusCode WebsocketInfoHandler::HandleRequest(const boost::beast::http::request<boost::beast::http::string_body> req,
                                               boost::beast::http::response<boost::beast::http::dynamic_body>& res) {
  std::vector<std::string> channels = state_->GetChannels();
  std::string body = "{ \"channels\": [";
  std::string chan_list = "";
  for (auto it : channels) {
	  chan_list += it + ", ";
  }
  if (chan_list.size() > 0) {
	  chan_list.resize(chan_list.size() - 2);
  }
  body += chan_list + "]}\n\n";
  boost::beast::ostream(res.body()) << body;
  res.result(OK);
  res.set(boost::beast::http::field::content_type, "application/json");
  LOG(info) << "Sent existing channels";
  res.prepare_payload();
  return OK;
}