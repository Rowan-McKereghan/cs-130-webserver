#include "request.h"

#include <boost/algorithm/string.hpp>
#include <iostream>
#include <sstream>
#include <string>

#include "config_parser.h"
#include "logger.h"

using namespace std;

Request::Request(const string& req_str) : raw_request(req_str) {
  istringstream request_stream(req_str);
  string line;
  // Parse the first line (Request-Line)
  if (getline(request_stream, line)) {
    istringstream line_stream(line);
    line_stream >> this->method >> this->uri;
    string http_version_string;
    line_stream >> http_version_string;
    if (http_version_string == "HTTP/1.1") {
      this->http_version = 1;
    } else {
      // TODO: Handle other HTTP versions or default to 1.0
      LOG(trace) << "Unsupported HTTP version string: " << http_version_string;
      this->http_version = 0;
    }
  }

  // Parse the header fields
  while (getline(request_stream, line) && !line.empty() && line != "\r") {
    HTTPHeader header;
    size_t colon_pos = line.find(':');
    if (colon_pos != string::npos) {
      header.name = line.substr(0, colon_pos);
      colon_pos++;  // Move past the colon

      // Allow whitespace after the colon
      while (colon_pos < line.length() && isspace(line[colon_pos])) {
        colon_pos++;
      }

      header.value = line.substr(colon_pos);

      // Check if line ends with "\r\n"
      // RFC7230, § 3.5 recomends allowing just a \n to terminate while parsing
      // We still want to log if this is occurs as it is technically not a valid
      // HTTP message
      if (line.back() != '\n' ||
          line.length() >= 2 && line[line.length() - 2] != '\r') {
        LOG(trace) << "Header line does not end with '\\r\\n': " << line;
      }

      // Trim any "\r" or "\n" characters from the header value
      header.value.erase(remove(header.value.begin(), header.value.end(), '\r'),
                         header.value.end());
      header.value.erase(remove(header.value.begin(), header.value.end(), '\n'),
                         header.value.end());

      this->headers.push_back(header);
    }
  }
}