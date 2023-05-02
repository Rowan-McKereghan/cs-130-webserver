#ifndef REQUEST_PROCESSOR_H
#define REQUEST_PROCESSOR_H

#include "request.h"
#include "response.h"
#include "request_handler_echo.h"
#include "request_handler_static.h"

#include <string>

// TODO - delete
using namespace std;

class RequestProcessor {
public:
	void ParseRequest(std::string req, Response &res);
};

#endif