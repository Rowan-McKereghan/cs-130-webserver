#include "crud_filesystem_manager.h"

CrudFileSystemManager::CrudFileSystemManager(){};

// Placeholders for now
bool CrudFileSystemManager::CreateDir(boost::filesystem::path path) { return true; }
bool CrudFileSystemManager::CreateFile(boost::filesystem::path path, std::string content) { return true; }
bool CrudFileSystemManager::ReadFile(boost::filesystem::path path, std::string& out) { return true; }
bool CrudFileSystemManager::WriteFile(boost::filesystem::path path, std::string content) { return true; }
bool CrudFileSystemManager::DeleteFile(boost::filesystem::path path) { return true; }
bool CrudFileSystemManager::ListFiles(boost::filesystem::path path) { return true; }