#ifndef CRUD_HANDLER_H
#define CRUD_HANDLER_H
#include <boost/beast/http.hpp>
#include <boost/filesystem.hpp>
#include <list>
#include <unordered_map>

#include "I_request_handler.h"

namespace http = boost::beast::http;

// Request Handler for basic CRUD (Create, Read, Update, and Delete) api
class CrudHandler : public I_RequestHandler {
 public:
  CrudHandler(std::string uri_path, unordered_map<std::string, std::list<int>>& entity_to_id);
  StatusCode HandleRequest(const http::request<http::string_body> req, http::response<http::dynamic_body>& res);

  // Placeholders for now
  StatusCode HandlePost(http::response<http::dynamic_body>& res);
  StatusCode HandleGet(http::response<http::dynamic_body>& res);
  StatusCode HandlePut(http::response<http::dynamic_body>& res);
  StatusCode HandleDelete(http::response<http::dynamic_body>& res);

 private:
  // fills fields in res according to status_code (assumes status code is not OK)
  void ReturnError(StatusCode status_code, http::response<http::dynamic_body>& res);

  boost::filesystem::path file_path_;
  
  //Placeholder, may change as we implement the functionality using this
  unordered_map<std::string, std::list<int>>* entity_to_id_;  // map of entity name to list of IDs
};

#endif