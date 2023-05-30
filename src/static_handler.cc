#include "static_handler.h"

#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/filesystem.hpp>
#include <boost/iostreams/stream.hpp>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>

#include "logger.h"
#include "privileged_dirs.h"

const int MAX_FILE_SIZE = 10485760;

StaticHandler::StaticHandler(std::string file_path, const std::string& client_ip) : file_path_(boost::filesystem::path(file_path)) {
	this->client_ip = client_ip;
}

// surprising not a library function for this in boost::filesystem
// please replace if someone can find one
static bool IsParentDir(const boost::filesystem::path &parent_path, const boost::filesystem::path &child_path) {
  int p_depth = std::distance(parent_path.begin(), parent_path.end());
  int c_depth = std::distance(child_path.begin(), child_path.end());

  // return false when parent has more directory levels than child
  if (p_depth > c_depth) {
    return false;
  }

  auto parent_end = parent_path.end();
  auto child_end = child_path.end();

  // loop to compare parent and child paths at each level
  for (auto parent_it = parent_path.begin(), child_it = child_path.begin(); parent_it != parent_end;
       ++parent_it, ++child_it) {
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

StatusCode StaticHandler::SetHeaders(const boost::beast::http::request<boost::beast::http::string_body> req,
                                     boost::beast::http::response<boost::beast::http::dynamic_body> &res) {
  res.version(req.version());
  if (IsInPrivilegedDirectory(file_path_)) {
    LOG(error) << file_path_ << " is an in a privileged directory";
    res.result(FORBIDDEN);
    return FORBIDDEN;
  }

  if (!boost::filesystem::is_regular_file(file_path_)) {
    // im not sure about the case when we serve
    // symlinks... does it work the same?
    LOG(error) << "File not found at: " << file_path_;
    res.result(NOT_FOUND);
    return NOT_FOUND;
  }

  if (boost::filesystem::file_size(file_path_) > MAX_FILE_SIZE - 1) {
    LOG(error) << "File requested too large (more than 10MB)";
    res.result(BAD_REQUEST);
    return BAD_REQUEST;
  }

  LOG(info) << "Serving file " << file_path_.string();

  res.result(boost::beast::http::status::ok);

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
    content_type = "application/octet-stream";  // download file as binary if it exists but
                                                // our server doesn't support its type
  }

  res.set(boost::beast::http::field::content_type, content_type);
  res.set(boost::beast::http::field::server, "webserver");
  return OK;
}

StatusCode StaticHandler::HandleRequest(const boost::beast::http::request<boost::beast::http::string_body> req,
                                        boost::beast::http::response<boost::beast::http::dynamic_body> &res) {
  file_path_ = boost::filesystem::current_path() / file_path_;
  StatusCode status_code_ = SetHeaders(req, res);

  if (status_code_ != OK) {
    boost::beast::http::dynamic_body::value_type err_body;
    std::string status_string = std::to_string(status_code_) + " ";
    auto it = status_code_map_.find(status_code_);
    if (it != status_code_map_.end()) {
      status_string += it->second.first;
    } else {
      status_string += "Unknown Status Code";
      LOG(warning) << "Unknown Status Code Found: " << std::to_string(status_code_);
    }

    boost::beast::ostream(res.body()) << status_string;
    LOG(info) << log_magic << "Response code: " << status_code_ << " Request path: " << req.target() << " Request IP: " << client_ip << " Handler Name: " << handler_name;
    res.prepare_payload();
    res.set(boost::beast::http::field::content_type, "text/HTML");
    return status_code_;
  }

  std::ifstream file_stream(file_path_.string(), std::ios::in | std::ios::binary);
  file_stream.seekg(0, std::ios::beg);

  // Write the file contents to the response buffer in chunks
  // 1024B is what boost recommends.
  char buffer_arr[1024];

  // check for eof to ensure last incomplete chunk written to ostream
  while (!file_stream.eof() && file_stream.good()) {
    file_stream.read(buffer_arr, sizeof(buffer_arr));
    boost::beast::ostream(res.body()) << boost::beast::string_view(buffer_arr, file_stream.gcount());
    if (file_stream.bad()) {
      LOG(error) << "I/O error reading file " << file_path_ << " into buffer";
      return INTERNAL_SERVER_ERROR;
    }
  }

  LOG(info) << log_magic << "Response code: " << status_code_ << " Request path: " << req.target() << " Request IP: " << client_ip << " Handler Name: " << handler_name;
  res.prepare_payload();
  res.erase(boost::beast::http::field::transfer_encoding);

  return status_code_;
}