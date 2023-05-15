#include "static_handler.h"

#include <boost/asio.hpp>
#include <boost/filesystem.hpp>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>

#include "logger.h"
#include "privileged_dirs.h"
#include "response.h"

const int MAX_FILE_SIZE = 10485760;

StaticHandler::StaticHandler(std::string file_path)
    : file_path_(boost::filesystem::path(file_path)) {}

// surprising not a library function for this in boost::filesystem
// please replace if someone can find one
static bool IsParentDir(const boost::filesystem::path &parent_path,
                        const boost::filesystem::path &child_path) {
  int p_depth = std::distance(parent_path.begin(), parent_path.end());
  int c_depth = std::distance(child_path.begin(), child_path.end());

  // return false when parent has more directory levels than child
  if (p_depth > c_depth) {
    return false;
  }

  auto parent_end = parent_path.end();
  auto child_end = child_path.end();

  // loop to compare parent and child paths at each level
  for (auto parent_it = parent_path.begin(), child_it = child_path.begin();
       parent_it != parent_end; ++parent_it, ++child_it) {
    // found a part of the parent path that doesn't match child path
    // before exhausting all of parent path, so return false
    if (*parent_it != *child_it) {
      return false;
    }
  }

  // exhausted parent path, so return true
  return true;
}

static bool IsInPrivilegedDirectory(const boost::filesystem::path &file_path) {
  boost::filesystem::path path = boost::filesystem::absolute(file_path);

  // privileged_dirs is set in privileged_dirs.h
  for (const auto &privileged_dir : privileged_dirs) {
    boost::filesystem::path privileged_path(privileged_dir);
    privileged_path = boost::filesystem::absolute(privileged_path);

    // both path and privileged_path will be absolute to allow for
    // straighforward comparison without factoring in . and ..
    if (IsParentDir(privileged_path, path)) {
      // the requested file is located in a privileged directory
      return true;
    }
  }

  // the requested file is not located in a privileged directory
  return false;
}

int StaticHandler::SetHeaders(const Request &req, Response &res) {
  if (IsInPrivilegedDirectory(file_path_)) {
    LOG(error) << file_path_ << " is an in a privileged directory";
    res.set_error_response(FORBIDDEN);
    return -1;
  }

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

void StaticHandler::HandleRequest(const Request &req, Response &res) {
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