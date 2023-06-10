#include "websocket_info_handler_factory.h"
#include "global_websocket_state.h"


WebsocketInfoHandlerFactory::WebsocketInfoHandlerFactory(std::shared_ptr<GlobalWebsocketState> state) : state_(state) {}

WebsocketInfoHandler* WebsocketInfoHandlerFactory::CreateHandler(std::string& uri_path) {
	return new WebsocketInfoHandler(state_);
}