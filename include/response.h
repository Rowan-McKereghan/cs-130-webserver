#ifndef RESPONSE_H
#define RESPONSE_H

#include "HTTPHeader.h"
#include <boost/asio.hpp>
#include <boost/filesystem.hpp>
#include <vector>
#include <string>

const int OK = 200;
const int BAD_REQUEST = 400;
const int NOT_FOUND = 404;
const int INTERNAL_SERVER_ERROR = 500;
const int MAX_FILE_SIZE = 10485760; //10 MB for now

class Response {
public:
	// Add more status codes as needed
	  // Add more status codes as needed
  	int status_code;
  	std::vector<HTTPHeader> headers;
  	std::string data;
  	boost::asio::ip::tcp::socket* socket_;

  	// New constructor that takes a socket object
  	Response(boost::asio::ip::tcp::socket* socket) : socket_(socket) {}

  	std::string generate_http_response();
  	void write_to_socket(const std::string& message);
	bool is_serving_file = false;
};

#endif