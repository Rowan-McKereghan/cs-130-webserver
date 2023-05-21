#include "crud_filesystem_manager.h"

#include <boost/beast/core.hpp>
#include <iostream>
#include <vector>

#include "logger.h"

CrudFileSystemManager::CrudFileSystemManager(){};

bool CrudFileSystemManager::CreateDir(boost::filesystem::path path) {
  boost::system::error_code ec;
  if (boost::filesystem::exists(path) && boost::filesystem::is_directory(path)) {
    return true;
  }
  LOG(info) << "Creating Directory at " << path;
  boost::filesystem::create_directory(path, ec);
  return ec == boost::system::errc::success;
}

bool CrudFileSystemManager::CreateFile(boost::filesystem::path path, std::string content) {
  if (boost::filesystem::exists(path)) {
    LOG(warning) << "File " << path << " already exists";
    return false;  // don't allow creating file that already exits
  }
  std::ofstream file(path.string());
  if (file) {
    file << content;
    file.close();
    return true;
  } else {
    LOG(warning) << "Failed to open " << path;
    return false;
  }
}

bool CrudFileSystemManager::ReadFile(boost::filesystem::path path,
                                     boost::beast::http::response<boost::beast::http::dynamic_body>& res) {
  std::ifstream file_stream(path.string(), std::ios::in | std::ios::binary);
  if (!file_stream.is_open()) {
    LOG(warning) << "Unable to open " << path.string();
    return false;
  }
  file_stream.seekg(0, std::ios::beg);

  // Write the file contents to the response buffer in chunks
  // 1024B is what boost recommends.
  char buffer_arr[1024];
  // check for eof to ensure last incomplete chunk written to ostream
  while (!file_stream.eof() && file_stream.good()) {
    file_stream.read(buffer_arr, sizeof(buffer_arr));
    boost::beast::ostream(res.body()) << boost::beast::string_view(buffer_arr, file_stream.gcount());
    if (file_stream.bad()) {
      LOG(error) << "I/O error reading file " << path << " into buffer";
      return false;
    }
  }
  return true;
}

bool CrudFileSystemManager::WriteFile(boost::filesystem::path path, std::string content) {
  if (!boost::filesystem::exists(path)) {
    LOG(warning) << "File " << path << " does not exists";
    return false;  // only write to file that exists already
  }
  std::ofstream file(path.string());
  if (file) {
    file << content;
    file.close();
    return true;
  } else {
    LOG(warning) << "Failed to open " << path;
    return false;
  }
}

bool CrudFileSystemManager::DeleteFile(boost::filesystem::path path) {
  boost::system::error_code ec;
  if (!boost::filesystem::exists(path)) {
    LOG(warning) << "Failed to delete " << path << ", it doesn't exist";
    return false;
  }
  boost::filesystem::remove(path, ec);
  std::vector<std::string> dir_content;
  ListFiles(path.parent_path(), dir_content);
  if (ec == boost::system::errc::success && dir_content.size() == 0) {
    // If removed last file for an entity delete that entity's dir
    boost::filesystem::remove(path.parent_path(), ec);
    return true;
  }
  return ec == boost::system::errc::success;
}

bool CrudFileSystemManager::ListFiles(boost::filesystem::path path, std::vector<std::string>& out_vect) {
  out_vect.clear();  // clear entries in case out_vect is full
  if (!boost::filesystem::is_directory(path)) {
    LOG(warning) << path << " is not a directory";
    return false;
  }

  for (boost::filesystem::directory_iterator it(path); it != boost::filesystem::directory_iterator(); ++it) {
    if (boost::filesystem::is_regular_file(*it)) {
      out_vect.push_back(it->path().filename().string());
    }
  }
  return true;
}