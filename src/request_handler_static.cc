#include "request_handler_static.h"

#include <boost/asio.hpp>
#include <fstream>
#include <map>
#include <sstream>
#include "logger.h"

RequestHandlerStatic::RequestHandlerStatic(string dir) { base_dir = dir; }

static std::string get_content_type(const std::string& extension) {
    static const std::unordered_map<std::string, std::string> mime_types = {
      {".html", "text/html"},
      {".htm", "text/html"},
      {".css", "text/css"},
      {".js", "application/javascript"},
      {".json", "application/json"},
      {".xml", "application/xml"},
      {".png", "image/png"},
      {".jpg", "image/jpeg"},
      {".jpeg", "image/jpeg"},
      {".gif", "image/gif"},
      {".svg", "image/svg+xml"},
      {".ico", "image/x-icon"},
      {".pdf", "application/pdf"}
      // Add more MIME types as needed
    };

    auto it = mime_types.find(extension);
    if (it != mime_types.end()) {
      return it->second;
    } else {
      return "";
    }
  }

void RequestHandlerStatic::HandleRequest(const Request& req, Response& res) {
  // Get the file path from the request URI
  std::string file_path = "/usr/src/projects/ctrl-c-ctrl-v/src/static/integration_test.html"; // base_dir + req.uri;

  LOG(info) << file_path;

  // Open the file
  std::ifstream file_stream(file_path, std::ios::in | std::ios::binary);
  if (!file_stream) {
    // File not found
    res.status_code = NOT_FOUND;
    res.data = "File not found";
  } else {
    // Get the file size
    file_stream.seekg(0, std::ios::end);
    std::size_t content_length = file_stream.tellg();
    file_stream.seekg(0, std::ios::beg);

    // Set the Content-Length header
    res.headers.emplace_back("Content-Length", std::to_string(content_length));

    // Set the Content-Type header based on the file extension
    std::size_t dot_pos = file_path.rfind(".");
    if (dot_pos != std::string::npos) {
      std::string extension = file_path.substr(dot_pos);
      std::string content_type = get_content_type(extension);
      if (!content_type.empty()) {
        res.headers.emplace_back("Content-Type", content_type);
      }
    }

    // Write the headers to the socket
    std::string response = res.generate_http_response();
    res.write_to_socket(response);

    // Write the file contents to the socket in chunks
    char buffer[8192];
    while (file_stream) {
      file_stream.read(buffer, sizeof(buffer));
      res.write_to_socket(std::string(buffer, file_stream.gcount()));
    }
  }
}
