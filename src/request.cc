#include "request.h"

#include <boost/algorithm/string.hpp>
#include <iostream>
#include <sstream>
#include <string>

#include "config_parser.h"
#include "logger.h"

using namespace std;

Request Request::ParseHTTPRequest(const string &req) {
  Request req_obj(req);
  istringstream request_stream(req);
  string line;
  // Parse the first line (Request-Line)
  if (getline(request_stream, line)) {
    istringstream line_stream(line);
    line_stream >> req_obj.method >> req_obj.uri;
    string http_version_string;
    line_stream >> http_version_string;
    if (http_version_string == "HTTP/1.1") {
      req_obj.http_version = 1;
    } else {
      // TODO: Handle other HTTP versions or default to 1.0
      LOG(trace) << "Unsupported HTTP version string: " << http_version_string;
      req_obj.http_version = 0;
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
      while (isspace(line[colon_pos]) && colon_pos < line.length()) {
        colon_pos++;
      }

      header.value = line.substr(colon_pos);

      // Check if line ends with "\r\n"
      // RFC7230, § 3.5 recomends allowing just a \n to terminate while parsing
      // We still want to log if this is occurs as it is technically not a valid
      // HTTP message
      if (line.back() != '\n' || line[line.length() - 2] != '\r') {
        LOG(trace) << "Header line does not end with '\\r\\n': " << line;
      }

      // Trim any "\r" or "\n" characters from the header value
      header.value.erase(remove(header.value.begin(), header.value.end(), '\r'),
                         header.value.end());
      header.value.erase(remove(header.value.begin(), header.value.end(), '\n'),
                         header.value.end());

      req_obj.headers.push_back(header);
    }
  }

  return req_obj;
}