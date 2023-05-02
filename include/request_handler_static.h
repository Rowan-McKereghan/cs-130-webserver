#ifndef REQUEST_HANDLER_STATIC_H
#define REQUEST_HANDLER_ECHO_H

#include "request_handler.h"

class RequestHandlerStatic : public I_RequestHandler {
public:
	RequestHandlerStatic(string base_dir);
	void HandleRequest(const Request &req, Response &res);
private:
	string base_dir;
};

#endif