#include "response.h"

#include <ctime>
#include <fstream>
#include <sstream>

#include "logger.h"


std::string Response::generate_http_response() {
  std::ostringstream res;

  auto it = status_code_map.find(status_code);
  auto status_description = it != status_code_map.end() ? it->second : "Unknown Status Code";

  // Generate the status line
  res << "HTTP/1.1 " << status_code << " "
      << status_description << "\r\n";
  // Generate the date header
  time_t t;
  struct tm* myTime;
  char date[100];
  t = time(NULL);
  myTime = gmtime(&t);
  strftime(date, sizeof(date), "Date: %a, %d %b %G %T GMT", myTime);
  res << date << "\r\n";
  // Add server header
  res << "Server: webserver\r\n";
  // Add the other headers
  for (const auto& header : headers) {
    res << header.name << ": " << header.value << "\r\n";
  }
  // Add an empty line to separate headers from the body
  res << "\r\n";
  // Add the response body
  if (!is_serving_file) {
    res << data;
  }
  return res.str();
}

void Response::write_to_socket(const std::string& message) {
  boost::system::error_code ec;
  boost::asio::write(*this->socket_, boost::asio::buffer(message), ec);
  if (ec) {
    log_error(ec, "Failed to write headers to socket");
    return;
  }
}