#include "crud_handler.h"

#include <boost/beast/core.hpp>

#include "crud_filesystem_manager.h"
#include "logger.h"

const int MAX_FILE_SIZE = 10485760;  // 10MB

CrudHandler::CrudHandler(std::string file_path, unordered_map<std::string, std::unordered_set<int>>& entity_to_id,
                         CrudFileSystemManager* manager)
    : file_path_(boost::filesystem::path(file_path)), entity_to_id_(&entity_to_id), manager_(manager) {
  LOG(info) << "creating crud handler";
}

void CrudHandler::ReturnError(StatusCode status_code, http::response<http::dynamic_body>& res) {
  res.result(status_code);
  std::string status_string = std::to_string(status_code) + " ";
  auto it = status_code_map_.find(status_code);
  if (it != status_code_map_.end()) {
    status_string += it->second.first;
  } else {
    status_string += "Unknown Status Code";
    LOG(warning) << "Unknown Status Code Found: " << std::to_string(status_code);
  }
  status_string += "\n\n";
  boost::beast::ostream(res.body()) << status_string;
  res.prepare_payload();
  res.set(boost::beast::http::field::content_type, "text/HTML");
}

// TODO: Possibly change this if we change the type of entity_to_id_
int CrudHandler::GetNextID(std::string entity) {
  auto it = entity_to_id_->find(entity);
  if (it == entity_to_id_->end()) {
    return 1;  // if set is empty return 1
  }

  // otherwise return next lowest available id
  int next_id = 1;
  while (next_id <= it->second.size()) {
    if (it->second.find(next_id) == it->second.end()) {
      return next_id;
    }
    next_id++;
  }
  return next_id;
}

int CrudHandler::GetPathLen() {
  int length = 0;
  boost::filesystem::path path = file_path_;
  while (!path.empty() && path != path.root_path()) {
    path = path.parent_path();
    length++;
  }
  return length;
}

std::pair<std::string, int> CrudHandler::ParseTarget() {
  int path_len = GetPathLen();  // Can only be 3 or 2 because otherwise HandleRequest would throw an error before
                                // ParseTarget called
  int id = -1;
  std::string entity = file_path_.string();  // if path_len is 2 this is enough
  std::string file_name = file_path_.filename().string();
  if (path_len == 3 &&
      std::strspn(file_name.c_str(), "0123456789") == file_name.length()) {  // ID is in path, and ID only contains #'s
    id = stoi(file_path_.filename().string());
    entity = file_path_.parent_path().string();
  } else if (path_len == 3) {  // ID is in path, and ID contains numbers -- bad request
    entity = "";
  }
  return std::pair<std::string, int>(entity, id);
}

// Post method creates stores the content of the request into a file corresponding to next available ID for an entity
// Returns the ID of the newly created entity EX. {"id": 1}
// Returns BAD_REQUEST if ID is included in POST request
// Returns INTERNAL_SERVER_ERROR if one of the file operations fails
StatusCode CrudHandler::HandlePost(const http::request<http::string_body> req,
                                   http::response<http::dynamic_body>& res) {
  std::pair<std::string, int> entity = ParseTarget();
  // Return bad request if id is included in POST request
  if (entity.second != -1) {
    LOG(warning) << "id incorrectly included in POST request";
    return BAD_REQUEST;
  }

  int id = GetNextID(entity.first);
  boost::filesystem::path entity_dir = boost::filesystem::current_path() / entity.first;
  boost::filesystem::path full_path = entity_dir / std::to_string(id);
  // create entity directory if it does not already exist
  if (!manager_->CreateDir(entity_dir)) {
    LOG(warning) << "Failed to create directory: " << entity_dir;
    return INTERNAL_SERVER_ERROR;
  }

  // create file
  if (!manager_->CreateFile(full_path, req.body())) {
    LOG(warning) << "Failed to create file: " << full_path;
    return INTERNAL_SERVER_ERROR;
  }
  // update response body with id
  std::string body = "{\"id\": " + std::to_string(id) + "}\n\n";
  boost::beast::ostream(res.body()) << body;

  LOG(info) << "Successfully created file: " << full_path;

  // Add in id to list and make sure list is sorted
  (*entity_to_id_)[entity.first].insert(id);
  LOG(info) << "Successfully added id  " << id << " to entity " << entity.first;
  return OK;
}

// Put method updates the data for a specific instance of an entity based on its ID
// Returns the ID of the updated entity EX. {"id": 1}
// Returns BAD_REQUEST if id is not included in request
// Returns NOT_FOUND if instance of entity does not exists
// Returns INTERNAL_SERVER_ERROR if file writing operation fails
StatusCode CrudHandler::HandlePut(const http::request<http::string_body> req, http::response<http::dynamic_body>& res) {
  std::pair<std::string, int> entity = ParseTarget();
  // Return bad request if id is included in POST request
  if (entity.second == -1) {
    LOG(warning) << "entity id not specified for PUT request";
    return BAD_REQUEST;
  }

  int id = entity.second;

  // check if entity with given id does not exist return 404 error
  if (entity_to_id_->find(entity.first) == entity_to_id_->end() ||
      (*entity_to_id_)[entity.first].find(id) == (*entity_to_id_)[entity.first].end()) {
    LOG(warning) << "Entity: " << entity.first << " not found with id " << id;
    return NOT_FOUND;
  }

  boost::filesystem::path full_path = boost::filesystem::current_path() / entity.first / std::to_string(id);
  // Update file
  if (!manager_->WriteFile(full_path, req.body())) {
    LOG(warning) << "Failed to update file: " << full_path;
    return INTERNAL_SERVER_ERROR;
  }

  // update response body with id
  std::string body = "{\"id\": " + std::to_string(id) + "}\n\n";
  boost::beast::ostream(res.body()) << body;

  LOG(info) << "Successfully updated file: " << full_path;

  return OK;
}

// Delete method deletes the data for a specific instance of an entity based on its ID
// Deletes json file and removes ID from set of IDs for the entity
// Returns the ID of the deleted entity EX. {"id": 1}
// Returns BAD_REQUEST if id is not included in request
// Returns NOT_FOUND if instance of entity does not exists
// Returns INTERNAL_SERVER_ERROR if DeleteFile operation fails
StatusCode CrudHandler::HandleDelete(http::response<http::dynamic_body>& res) {
  std::pair<std::string, int> entity = ParseTarget();
  // Return bad request if id is included in DELETE request
  if (entity.second == -1) {
    LOG(warning) << "entity id not specified for DELETE request";
    return BAD_REQUEST;
  }

  int id = entity.second;

  // if entity with given id does not exist return 404 error
  if (entity_to_id_->find(entity.first) == entity_to_id_->end() ||
      (*entity_to_id_)[entity.first].find(id) == (*entity_to_id_)[entity.first].end()) {
    LOG(warning) << "Entity: " << entity.first << " not found with id " << id;
    return NOT_FOUND;
  }

  boost::filesystem::path full_path = boost::filesystem::current_path() / entity.first / std::to_string(id);
  // Update file
  if (!manager_->DeleteFile(full_path)) {
    LOG(warning) << "Failed to delete file: " << full_path;
    return INTERNAL_SERVER_ERROR;
  }

  // update response body with deleted id
  std::string body = "{\"id\": " + std::to_string(id) + "}\n\n";
  boost::beast::ostream(res.body()) << body;

  LOG(info) << "Successfully deleted file: " << full_path;

  // Remove id from set
  (*entity_to_id_)[entity.first].erase(id);
  LOG(info) << "Successfully removed id  " << id << " from entity " << entity.first;
  return OK;
}

// GET method returns information about either a specific entity or all instances of an entity
// If ID is included returns the data corresponding to the requested entity
// Returns NOT_FOUND if instance of entity does not exists
// If ID is not included returns a list of all files for a given entity Ex. [1, 2, 3]
// Returns empty list if entity does not exist
// Returns INTERNAL_SERVER_ERROR if file reading or file listing operations fail
StatusCode CrudHandler::HandleGet(http::response<http::dynamic_body>& res) {
  std::pair<std::string, int> entity = ParseTarget();
  // If id is included retrive information for specific instance of entity
  if (entity.second != -1) {
    int id = entity.second;
    // check if entity with given id does not exist return 404
    if (entity_to_id_->find(entity.first) == entity_to_id_->end() ||
        (*entity_to_id_)[entity.first].find(id) == (*entity_to_id_)[entity.first].end()) {
      LOG(warning) << "Entity: " << entity.first << " not found with id " << id;
      return NOT_FOUND;
    }

    boost::filesystem::path full_path = boost::filesystem::current_path() / entity.first / std::to_string(id);

    // read file contents into response body
    if (!manager_->ReadFile(full_path, res)) {
      LOG(warning) << "Failed to read file: " << full_path;
      return INTERNAL_SERVER_ERROR;
    }
  }
  // if id is not included, returns JSON list of valid IDs for the given entity
  else {
    // if entity doesn't exist or return the empty list
    if (entity_to_id_->find(entity.first) == entity_to_id_->end()) {
      boost::beast::ostream(res.body()) << "[]\n\n";
      return OK;
    }

    // read in filenames from entity directory
    boost::filesystem::path entity_dir = boost::filesystem::current_path() / entity.first;
    std::vector<std::string> file_names;
    if (!manager_->ListFiles(entity_dir, file_names)) {
      LOG(warning) << "Failed to list files from " << entity_dir;
      return INTERNAL_SERVER_ERROR;
    }

    // create JSON output of file names in entity directory
    // Ex: {[1, 2, 3]}
    std::string body = "[";
    for (std::string file : file_names) {
      body += file + ", ";
    }
    // remove trailing comma if it exists
    if (body.length() > 1) body.erase(body.length() - 2, 2);
    body += "]\n\n";

    // update response body
    boost::beast::ostream(res.body()) << body;
  }

  return OK;
}

StatusCode CrudHandler::HandleMethod(const http::request<http::string_body> req,
                                     http::response<http::dynamic_body>& res) {
  LOG(info) << "Request: " << http::to_string(req.method()) << " " << file_path_;

  switch (req.method()) {
    case http::verb::post:
      return HandlePost(req, res);
      break;
    case http::verb::get:
      return HandleGet(res);
      break;
    case http::verb::put:
      return HandlePut(req, res);
      break;
    case http::verb::delete_:
      return HandleDelete(res);
      break;
    default:
      LOG(warning) << "Invalid HTTP request method";
      return BAD_REQUEST;
      break;
  }
}

StatusCode CrudHandler::InitRoot(const http::request<http::string_body> req, http::response<http::dynamic_body>& res) {
  res.version(req.version());
  res.set(boost::beast::http::field::server, "webserver");

  if ((GetPathLen() != 2 && GetPathLen() != 3) || ParseTarget().first == "") {
    LOG(warning) << "Bad request to CRUD handler, path = " << file_path_;
    return BAD_REQUEST;
  }

  boost::filesystem::path root = file_path_;
  while (root.parent_path().string() != "/") {  // find root dir
    root = root.parent_path();
  }
  root = boost::filesystem::current_path() / root;
  if (!(manager_->CreateDir(root))) {  // Create root dir if doesn't exist
    LOG(error) << "Failed to Open CRUD root at " << root.string();
    return INTERNAL_SERVER_ERROR;
  }
  return OK;
}

StatusCode CrudHandler::HandleRequest(const http::request<http::string_body> req,
                                      http::response<http::dynamic_body>& res) {
  StatusCode status_code_ = InitRoot(req, res);

  if (status_code_ == OK) {  // Don't attempt to deal with method if an error already occurred
    status_code_ = HandleMethod(req, res);
  }

  // set response to error page if status isn't OK
  if (status_code_ != OK) {
    ReturnError(status_code_, res);
    return status_code_;
  }

  // Temporary to return a response to confirm this handler is running
  res.result(status_code_);
  // boost::beast::ostream(res.body()) << "testing crud handler setup";
  res.prepare_payload();
  res.set(boost::beast::http::field::content_type, "application/json");

  return status_code_;
}