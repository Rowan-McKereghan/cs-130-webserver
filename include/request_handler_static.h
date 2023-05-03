#ifndef REQUEST_HANDLER_STATIC_H
#define REQUEST_HANDLER_ECHO_H

#include "request_handler.h"

#include <boost/filesystem.hpp>

class RequestHandlerStatic : public I_RequestHandler {
public:
	RequestHandlerStatic(boost::filesystem::path root);
	void HandleRequest(const Request &req, Response &res);
	int HandleRequestHelper(const Request &req, boost::filesystem::path &complete_path, Response &res);
private:
	boost::filesystem::path root_;
};

#endif