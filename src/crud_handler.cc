#include "crud_handler.h"

#include <boost/beast/core.hpp>

#include "crud_filesystem_manager.h"
#include "logger.h"

const int MAX_FILE_SIZE = 10485760;  // 10MB

CrudHandler::CrudHandler(std::string file_path, unordered_map<std::string, std::set<int>>& entity_to_id,
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

  //otherwise return next lowest available id
  int next_id = 1;
  for (int val : it->second) {  // check if an earlier ID has been deleted
    if (val > next_id) {
      return next_id;
    }
    next_id++;
  }
  return next_id;
}

std::pair<std::string, int> CrudHandler::ParseTarget() {
  int id = -1;
  std::string entity = file_path_.string();
  //Placeholder: wont work with non-single-digit IDs
  if (file_path_.filename().string().length() == 1) {  // ie last part of file is an ID #
    id = std::stoi(file_path_.filename().string());
    entity = file_path_.parent_path().string();
  }
  return std::pair<std::string, int>(entity, id);
}

// TODO: potentially update to use more descriptive error codes instead of only BAD_REQUEST
StatusCode CrudHandler::HandlePost(const http::request<http::string_body> req, http::response<http::dynamic_body>& res) {
  std::pair<std::string, int> entity = ParseTarget();
  //Return bad request if id is included in POST request
  if(entity.second!=-1){
    LOG(warning) << "id incorrectly included in POST request";
    return BAD_REQUEST;
  }

  int id = GetNextID(entity.first);
  boost::filesystem::path entity_dir = boost::filesystem::current_path() / entity.first;
  boost::filesystem::path full_path = entity_dir / std::to_string(id);
  // create entity directory if it does not already exist
  if(!manager_->CreateDir(entity_dir)){
    LOG(warning) << "Failed to create directory: " << entity_dir;
    return BAD_REQUEST;
  }

  // create file
  if(!manager_->CreateFile(full_path, req.body())){
    LOG(warning) << "Failed to create file: " << full_path;
    return BAD_REQUEST;
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

StatusCode CrudHandler::HandlePut(const http::request<http::string_body> req, http::response<http::dynamic_body>& res) { 
  std::pair<std::string, int> entity = ParseTarget();
  //Return bad request if id is included in POST request
  if(entity.second==-1){
    LOG(warning) << "entity id not specified for PUT request";
    return BAD_REQUEST;
  }

  int id = entity.second;

  //check if entity with given id does not exist return bad request error
  //As mentioned above may in the future find more descriptive error to give
  if(entity_to_id_->find(entity.first)==entity_to_id_->end() || (*entity_to_id_)[entity.first].find(id)== (*entity_to_id_)[entity.first].end()){
    LOG(warning) << "Entity: " << entity.first << " not found with id " << id;
    return BAD_REQUEST;
  }

  boost::filesystem::path full_path = boost::filesystem::current_path() / entity.first / std::to_string(id);
  // Update file
  if(!manager_->WriteFile(full_path, req.body())){
    LOG(warning) << "Failed to update file: " << full_path;
    return BAD_REQUEST;
  }

  // update response body with id
  std::string body = "{\"id\": " + std::to_string(id) + "}\n\n";
  boost::beast::ostream(res.body()) << body;

  LOG(info) << "Successfully updated file: " << full_path;

  return OK; 
}

// Placeholders for now
StatusCode CrudHandler::HandleGet(http::response<http::dynamic_body>& res) { return OK; }
StatusCode CrudHandler::HandleDelete(http::response<http::dynamic_body>& res) { return OK; }

StatusCode CrudHandler::HandleRequest(const http::request<http::string_body> req,
                                      http::response<http::dynamic_body>& res) {
  
  StatusCode status_code_;
  res.version(req.version());
  res.set(boost::beast::http::field::server, "webserver");
  boost::filesystem::path root = file_path_;
  while (root.parent_path().string() != "/") {  // find root dir
    root = root.parent_path();
  }
  root = boost::filesystem::current_path() / root;
  if (!(manager_->CreateDir(root))) {  // Create root dir if doesn't exist
    LOG(error) << "Failed to Open CRUD root at " << root.string();
  }
  LOG(info) << "Request: " << http::to_string(req.method()) << " " << file_path_;

  switch (req.method()) {
    case http::verb::post:
      status_code_ = HandlePost(req, res);
      break;
    case http::verb::get:
      status_code_ = HandleGet(res);
      break;
    case http::verb::put:
      status_code_ = HandlePut(req, res);
      break;
    case http::verb::delete_:
      status_code_ = HandleDelete(res);
      break;
    default:
      LOG(warning) << "Invalid HTTP request method";
      status_code_ = BAD_REQUEST;
      break;
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
  res.set(boost::beast::http::field::content_type, "text/plain");

  return status_code_;
}