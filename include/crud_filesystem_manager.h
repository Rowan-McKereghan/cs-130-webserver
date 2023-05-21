#ifndef CRUD_FILESYSTEM_MANAGER_H
#define CRUD_FILESYSTEM_MANAGER_H

#include <boost/beast/http.hpp>
#include <boost/filesystem.hpp>
#include <string>

// Used for all file operations on the crud filesystem
// Purpose is so to abstract the file interactions so we can make mocks or fakes for testing
class CrudFileSystemManager {
 public:
  CrudFileSystemManager();

  // Attempts to create directory at path if one doesn't already exist
  // returns true if successful or if path already is a dir, false otherwise
  virtual bool CreateDir(boost::filesystem::path path);

  // Attempts to create a file at path and write content to the file
  // returns true on success, false on failure including if file already exists
  virtual bool CreateFile(boost::filesystem::path path, std::string content);

  // Reads the content of the file at path into res
  virtual bool ReadFile(boost::filesystem::path path,
                        boost::beast::http::response<boost::beast::http::dynamic_body>& res);
  virtual bool WriteFile(boost::filesystem::path path, std::string content);

  // Attempts to delete the file at path (ie /root/entity/id)
  // if path is successfully deleted and is the last file at the parent dir removes that dir as well (ie /root/entity)
  // Only call on entity/ID files, the entity files will be automatically removed
  virtual bool DeleteFile(boost::filesystem::path path);

  // fills out_vect with the names of the files from the given directory
  // Returns whether path is to a directory or not
  virtual bool ListFiles(boost::filesystem::path path, std::vector<std::string>& out_vect);
};

#endif