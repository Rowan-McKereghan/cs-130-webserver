#include "request_processor.h"
#include <iostream> // TODO - delete

void RequestProcessor::ParseRequest(string req, Response &res) {
	// Should dynamically dispatch requestHandlerEcho or requestHandler
	// TODO - construct this Request object from our parsed request (current hardcoded)
	Request req_obj;
	req_obj.method = "get";
	req_obj.uri = "static/logger.cc";
	req_obj.http_version = 1;

	// TODO - get uri_start from req_obj.uri
	string uri_start = "static"; // TODO - replace placeholder with actual parsed implementation
								
	if (uri_start == "echo") {
		RequestHandlerEcho handler;
		handler.HandleRequest(req_obj, res);
	} else if (uri_start == "static") {
		// TODO - get base_dir from config parser
		string base_dir = "/src"; 
		RequestHandlerStatic handler(base_dir); 
		handler.HandleRequest(req_obj, res);
	}
}