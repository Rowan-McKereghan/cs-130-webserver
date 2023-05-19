#include "crud_handler.h"

#include <boost/beast/core.hpp>

#include "logger.h"

const int MAX_FILE_SIZE = 10485760;

CrudHandler::CrudHandler(std::string file_path) : file_path_(boost::filesystem::path(file_path)) {}

StatusCode CrudHandler::HandleRequest(const http::request<http::string_body> req,
                                      http::response<http::dynamic_body> &res) {
  // To Do
  res.result(OK);
  res.version(req.version());
  res.set(boost::beast::http::field::server, "webserver");
  boost::beast::ostream(res.body()) << "testing crud handler setup";
  res.prepare_payload();
  res.set(boost::beast::http::field::content_type, "text/plain");
  return OK;
}