#ifndef CRUD_FILESYSTEM_MANAGER_H
#define CRUD_FILESYSTEM_MANAGER_H

#include <boost/filesystem.hpp>
#include <string>

// Used for all file operations on the crud filesystem
// Purpose is so to abstract the file interactions so we can make mocks or fakes for testing
class CrudFileSystemManager {
 public:
  CrudFileSystemManager();
  virtual bool CreateDir(boost::filesystem::path path);
  virtual bool CreateFile(boost::filesystem::path path, std::string content);
  virtual bool ReadFile(boost::filesystem::path path, std::string& out);
  virtual bool WriteFile(boost::filesystem::path path, std::string content);
  virtual bool DeleteFile(boost::filesystem::path path);
};

#endif