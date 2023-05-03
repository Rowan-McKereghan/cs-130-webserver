#include "request_handler_static.h"

#include <boost/asio.hpp>
#include <boost/filesystem.hpp>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>

#include "logger.h"

RequestHandlerStatic::RequestHandlerStatic(boost::filesystem::path root)
    : file_path(root) {}

int RequestHandlerStatic::HandleRequestHelper(const Request &req,
                                              Response &res) {
  HTTPHeader content_type_header;
  content_type_header.name = "Content-Type";
  HTTPHeader content_length_header;
  content_length_header.name = "Content-Length";

  // TODO: refactor common response formatting (like 404) into a separate
  // function
  if (!file_path.string().compare("Invalid Path")) {
    res.data = "404 Not Found\r\n\r\n";
    res.status_code = NOT_FOUND;
    content_type_header.value = "text/HTML";
    content_length_header.value = res.data.length();
    std::vector<HTTPHeader> headers;
    res.headers.push_back(content_type_header);
    res.headers.push_back(content_length_header);
    res.status_code = NOT_FOUND;
    return -1;
  }

  // TODO: refactor common response formatting (like 404) into a separate
  // function
  if (!boost::filesystem::is_regular_file(
          file_path)) {  // im not sure about the case when we serve
                         // symlinks... does it work the same?
    LOG(error) << "File not found at: " << file_path;
    res.data = "404 Not Found\r\n\r\n";
    res.status_code = NOT_FOUND;
    content_type_header.value = "text/HTML";
    content_length_header.value = res.data.length();
    std::vector<HTTPHeader> headers;
    res.headers.push_back(content_type_header);
    res.headers.push_back(content_length_header);
    res.status_code = NOT_FOUND;
    return -1;
  }

  // TODO: refactor common response formatting (like 400) into a separate
  // function
  if (boost::filesystem::file_size(file_path) > MAX_FILE_SIZE - 1) {
    LOG(error) << "File requested too large (more than 10MB)";
    res.data = "400 Bad Request\r\n\r\n";
    HTTPHeader contentType;
    contentType.name = "Content-Type";
    contentType.value = "text/HTML";
    HTTPHeader contentLength;
    contentLength.name = "Content-Length";
    contentLength.value = res.data.length();
    std::vector<HTTPHeader> headers;
    headers.push_back(contentType);
    headers.push_back(contentLength);
    res.headers = headers;
    res.status_code = BAD_REQUEST;
    return -1;
  }

  LOG(info) << "Serving file " << file_path.string();
  res.is_serving_file = true;

  // set MIME type
  res.status_code = 200;
  if (!file_path.extension().string().compare(".txt")) {
    content_type_header.value = "text/plain";
  } else if (!file_path.extension().string().compare(".jpg") ||
             !file_path.extension().string().compare(".jpeg")) {
    content_type_header.value = "image/jpeg";
  } else if (!file_path.extension().string().compare(".png")) {
    content_type_header.value = "image/png";
  } else if (!file_path.extension().string().compare(".pdf")) {
    content_type_header.value = "application/pdf";
  } else if (!file_path.extension().string().compare(".html") ||
             !file_path.extension().string().compare(".htm")) {
    content_type_header.value = "text/html";
  } else if (!file_path.extension().string().compare(".zip")) {
    content_type_header.value = "application/zip";
  } else {
    content_type_header.value =
        "application/octet-stream";  // download file as binary if it exists but
                                     // our server doesn't support its type
  }
  res.headers.push_back(content_type_header);

  // find file size and set content length field
  content_length_header.value =
      std::to_string(boost::filesystem::file_size(file_path));
  res.headers.push_back(content_length_header);
  return 0;
}

void RequestHandlerStatic::HandleRequest(const Request &req, Response &res) {
  if (HandleRequestHelper(req, res) == -1) {
    return;
  }

  std::string response = res.generate_http_response();
  res.write_to_socket(response);

  std::ifstream file_stream(file_path.string(),
                            std::ios::in | std::ios::binary);
  file_stream.seekg(0, std::ios::beg);

  // Write the file contents to the socket in chunks
  // 8KB is a nice balance between file reading efficiency and socket writing
  // (OS buffer automatically fragments into packets)
  char buffer_arr[8192];
  while (file_stream) {
    file_stream.read(buffer_arr, sizeof(buffer_arr));
    boost::system::error_code ec;
    boost::asio::write(*res.socket_, boost::asio::buffer(buffer_arr), ec);
    if (ec) {
      log_error(ec, "Failed to write requested file to socket");
      return;
    }
  }
}