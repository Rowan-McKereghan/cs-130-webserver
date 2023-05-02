#ifndef REQUEST_H
#define REQUEST_H

#include <string>
#include <vector>
#include "HTTPHeader.h"

class Request {
public:
	std::string method;
	std::string uri;
	int http_version;
	std::vector<header> headers;
};

#endif