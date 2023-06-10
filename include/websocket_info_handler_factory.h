#ifndef WEBSOCKET_INFO_HANDLER_FACTORY_H
#define WEBSOCKET_INFO_HANDLER_FACTORY_H

#include "I_request_handler_factory.h"
#include "websocket_info_handler.h"

class WebsocketInfoHandlerFactory : public I_RequestHandlerFactory {
	public:
		WebsocketInfoHandlerFactory(std::shared_ptr<GlobalWebsocketState> state);
		WebsocketInfoHandler* CreateHandler(std::string& uri_path);
	private:
		std::shared_ptr<GlobalWebsocketState> state_;
};


#endif