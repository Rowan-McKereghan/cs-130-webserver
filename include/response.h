#ifndef RESPONSE_H
#define RESPONSE_H

#include "HTTPHeader.h"
#include <vector>
#include <string>

const int OK = 200;
const int BAD_REQUEST = 400;
const int NOT_FOUND = 404;
const int INTERNAL_SERVER_ERROR = 500;

class Response {
public:
	// Add more status codes as needed
	int status_code;
	std::vector<HTTPHeader> headers;
	std::string data;
	std::string generate_http_response();
};

#endif