#ifndef REQUEST_HANDLER_H
#define REQUEST_HANDLER_H

#include "request.h"
#include "response.h"
#include <string>

#include <iostream>
using namespace std;

class I_RequestHandler {
	public:
		virtual void HandleRequest(const Request &req, Response &res) = 0;
};

#endif