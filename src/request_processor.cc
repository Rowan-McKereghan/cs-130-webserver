#include "request_processor.h"

#include <iostream>
#include <sstream>

#include "config_parser.h"
#include "logger.h"
#include "request.h"

using namespace std;

#include <boost/algorithm/string.hpp>
#include <string>

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

std::string RequestProcessor::extract_uri_base_path(const std::string &uri) {
  std::string stripped_uri = uri;

  // Strip any query string from the URI
  std::size_t query_pos = stripped_uri.find('?');
  if (query_pos != std::string::npos) {
    stripped_uri = stripped_uri.substr(0, query_pos);
  }

  // Strip any trailing slashes from the URI
  boost::algorithm::trim_right_if(stripped_uri,
                                  boost::algorithm::is_any_of("/"));

  // Add back the final slash if the URI is empty or just "/"
  if (stripped_uri.empty() || stripped_uri == "/") {
    stripped_uri = "/";
  }

  return stripped_uri;
}

void RequestProcessor::RouteRequest(string req, ServingConfig serving_config,
                                    Response &res) {
  Request req_obj = Request::ParseHTTPRequest(req);
  auto uri_base_path = RequestProcessor::extract_uri_base_path(req_obj.uri);
  LOG(info) << "Client accessed URI: " << req_obj.uri << " "
            << uri_base_path;

  // Check if the URI matches any of the echo paths
  bool echo_path_matched = false;
  for (const auto &echo_path : serving_config.echo_paths) {
    if (uri_base_path == echo_path) {
      LOG(info) << "Request matched to echo path: " << echo_path;
      RequestHandlerEcho handler;
      handler.HandleRequest(req_obj, res);
      echo_path_matched = true;
      return;
    }
  }

  // Check if the URI matches any of the file paths
  bool file_path_matched = false;
  for (const auto &file_path : serving_config.static_file_paths) {
    if (uri_base_path.find(file_path.first) == 0) {
      LOG(info) << "Request matched to " << file_path.first
                << " serving files from: " << file_path.second;
      std::string absolute_file_path =
          file_path.second + uri_base_path.substr(file_path.first.length());
      LOG(info) << "Request matched to file" << absolute_file_path;
      boost::filesystem::path final_file_path(absolute_file_path);
      RequestHandlerStatic handler(final_file_path);
      handler.HandleRequest(req_obj, res);
      file_path_matched = true;
      return;
    }
  }

  // If neither echo path nor file path matches, return a 404 Not Found
  // response
  if (!echo_path_matched && !file_path_matched) {
    LOG(warning) << "Invalid URI accessed: " << req_obj.uri;
    res.data = "404 Not Found\r\n\r\n";
    HTTPHeader contentType;
    contentType.name = "Content-Type";
    contentType.value = "text/HTML";
    HTTPHeader contentLength;
    contentLength.name = "Content-Length";
    contentLength.value = std::to_string(res.data.size());
    vector<HTTPHeader> headers;
    headers.push_back(contentType);
    headers.push_back(contentLength);
    res.headers = headers;
    res.status_code = NOT_FOUND;
  }
}
