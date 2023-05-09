#include "request_handler_static.h"

#include <boost/asio.hpp>
#include <boost/filesystem.hpp>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>

#include "logger.h"
#include "response.h"

const int MAX_FILE_SIZE = 10485760;

RequestHandlerStatic::RequestHandlerStatic(boost::filesystem::path file_path)
    : file_path_(file_path) {}

int RequestHandlerStatic::SetHeaders(const Request &req, Response &res) {
  if (!boost::filesystem::is_regular_file(
          file_path_)) {  // im not sure about the case when we serve
                          // symlinks... does it work the same?
    LOG(error) << "File not found at: " << file_path_;
    res.set_error_response(NOT_FOUND);
    return -1;
  }

  if (boost::filesystem::file_size(file_path_) > MAX_FILE_SIZE - 1) {
    LOG(error) << "File requested too large (more than 10MB)";
    res.set_error_response(BAD_REQUEST);
    return -1;
  }

  LOG(info) << "Serving file " << file_path_.string();

  res.set_status_code(OK);

  // set MIME type
  std::string content_type;
  std::string file_path_str = file_path_.extension().string();
  if (file_path_str == ".txt") {
    content_type = "text/plain";
  } else if (file_path_str == ".jpg" || file_path_str == ".jpeg") {
    content_type = "image/jpeg";
  } else if (file_path_str == ".png") {
    content_type = "image/png";
  } else if (file_path_str == ".pdf") {
    content_type = "application/pdf";
  } else if (file_path_str == ".html" || file_path_str == ".htm") {
    content_type = "text/html";
  } else if (file_path_str == ".zip") {
    content_type = "application/zip";
  } else {
    content_type =
        "application/octet-stream";  // download file as binary if it exists but
                                     // our server doesn't support its type
  }

  res.add_header("Content-Type", content_type);
  res.add_header("Content-Length",
                 std::to_string(boost::filesystem::file_size(file_path_)));
  return 0;
}

void RequestHandlerStatic::HandleRequest(const Request &req, Response &res) {
  if (SetHeaders(req, res) == -1) {
    return;
  }

  if (!res.get_wrote_http_response()) {
    res.WriteHTTPResponse();
  }

  std::ifstream file_stream(file_path_.string(),
                            std::ios::in | std::ios::binary);
  file_stream.seekg(0, std::ios::beg);

  // Write the file contents to the socket in chunks
  // 8KB is a nice balance between file reading efficiency and socket writing
  // (OS buffer automatically fragments into packets)
  char buffer_arr[8192];
  while (file_stream) {
    file_stream.read(buffer_arr, sizeof(buffer_arr));
    res.WriteToSocket(boost::asio::buffer(buffer_arr, 8192));
  }
}