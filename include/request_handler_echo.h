#ifndef REQUEST_HANDLER_ECHO_H
#define REQUEST_HANDLER_ECHO_H

#include "request_handler.h"

class RequestHandlerEcho : public RequestHandler {
public:
	// No constructor needed since it is config invariant
	void HandleRequest(const Request &req, Response &res) override;
};

#endif