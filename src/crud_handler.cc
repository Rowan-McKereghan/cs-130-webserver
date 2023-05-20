#include "crud_handler.h"

#include <boost/beast/core.hpp>

#include "logger.h"

const int MAX_FILE_SIZE = 10485760;  // 10MB

CrudHandler::CrudHandler(std::string file_path, unordered_map<std::string, std::list<int>>& entity_to_id)
    : file_path_(boost::filesystem::path(file_path)), entity_to_id_(&entity_to_id) {}

// Placeholders for now
StatusCode CrudHandler::HandlePost(http::response<http::dynamic_body>& res) { return OK; }
StatusCode CrudHandler::HandleGet(http::response<http::dynamic_body>& res) { return OK; }
StatusCode CrudHandler::HandlePut(http::response<http::dynamic_body>& res) { return OK; }
StatusCode CrudHandler::HandleDelete(http::response<http::dynamic_body>& res) { return OK; }

StatusCode CrudHandler::HandleRequest(const http::request<http::string_body> req,
                                      http::response<http::dynamic_body>& res) {
  StatusCode status_code_;
  res.version(req.version());
  res.set(boost::beast::http::field::server, "webserver");

  LOG(info) << "Request: " << http::to_string(req.method()) << " " << file_path_;

  switch (req.method()) {
    case http::verb::post:
      status_code_ = HandlePost(res);
      break;
    case http::verb::get:
      status_code_ = HandleGet(res);
      break;
    case http::verb::put:
      status_code_ = HandlePut(res);
      break;
    case http::verb::delete_:
      status_code_ = HandleDelete(res);
      break;
    default:
      LOG(warning) << "Invalid HTTP request method";
      res.result(BAD_REQUEST);
      status_code_ = BAD_REQUEST;
      break;
  }

  // set response to error page if status isn't OK
  if (status_code_ != OK) {
    std::string status_string = std::to_string(status_code_) + " ";
    auto it = status_code_map_.find(status_code_);
    if (it != status_code_map_.end()) {
      status_string += it->second.first;
    } else {
      status_string += "Unknown Status Code";
      LOG(warning) << "Unknown Status Code Found: " << std::to_string(status_code_);
    }
    boost::beast::ostream(res.body()) << status_string;
    res.prepare_payload();
    res.set(boost::beast::http::field::content_type, "text/HTML");
    return status_code_;
  }

  // Temporary to return a response to confirm this handler is running
  res.result(status_code_);
  boost::beast::ostream(res.body()) << "testing crud handler setup";
  res.prepare_payload();
  res.set(boost::beast::http::field::content_type, "text/plain");

  return status_code_;
}