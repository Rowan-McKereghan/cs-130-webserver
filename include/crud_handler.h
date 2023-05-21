#ifndef CRUD_HANDLER_H
#define CRUD_HANDLER_H
#include <boost/beast/http.hpp>
#include <boost/filesystem.hpp>
#include <list>
#include <unordered_map>
#include <unordered_set>

#include "I_request_handler.h"
#include "crud_filesystem_manager.h"

namespace http = boost::beast::http;

// Request Handler for basic CRUD (Create, Read, Update, and Delete) api
class CrudHandler : public I_RequestHandler {
 public:
  CrudHandler(std::string uri_path, unordered_map<std::string, std::unordered_set<int>>& entity_to_id,
              CrudFileSystemManager* manager);
  StatusCode HandleRequest(const http::request<http::string_body> req, http::response<http::dynamic_body>& res);

  // get next available ID for the given entity
  int GetNextID(std::string entity);

  // Returns a pair with the entity name ("/root/entity") and the ID number in the request
  // If the request didn't include an ID returns -1 for the ID value
  std::pair<std::string, int> ParseTarget();

  // Placeholders for now
  StatusCode HandlePost(const http::request<http::string_body> req, http::response<http::dynamic_body>& res);
  StatusCode HandleGet(http::response<http::dynamic_body>& res);
  StatusCode HandlePut(const http::request<http::string_body> req, http::response<http::dynamic_body>& res);
  StatusCode HandleDelete(http::response<http::dynamic_body>& res);

 private:
  // fills fields in res according to status_code (assumes status code is not OK)
  void ReturnError(StatusCode status_code, http::response<http::dynamic_body>& res);

  boost::filesystem::path file_path_;

  // dependency injected in constructor; used for filesystem operations
  CrudFileSystemManager* manager_;

  // map of entity name to set of IDs
  // entity names in the form "/root/entity"
  unordered_map<std::string, std::unordered_set<int>>* entity_to_id_;
};

#endif