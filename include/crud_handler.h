#ifndef CRUD_HANDLER_H
#define CRUD_HANDLER_H

#include <boost/beast/http.hpp>
#include <boost/filesystem.hpp>

#include "I_request_handler.h"

namespace http = boost::beast::http;

// Request Handler for basic CRUD (Create, Read, Update, and Delete) api
class CrudHandler : public I_RequestHandler {
 public:
  CrudHandler(std::string uri_path);
  StatusCode HandleRequest(const http::request<http::string_body> req, http::response<http::dynamic_body>& res);

 private:
  boost::filesystem::path file_path_;
};

#endif