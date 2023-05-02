#include "response.h"
#include <ctime>
#include <sstream>
std::string Response::generate_http_response() {
  std::ostringstream res;
  // Generate the status line
  res << "HTTP/1.1 " << status_code << " OK\r\n";
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
  res << data;
  return res.str();
}