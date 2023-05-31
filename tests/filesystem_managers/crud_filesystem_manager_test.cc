#include "crud_filesystem_manager.h"

#include "gtest/gtest.h"

// Tests for the operations that don't involve creating/deleting/writing to files
// Testing file operations will be done in integration tests
TEST(CrudFileSystemManager, TestCreateDirAlreadyExist) {
  CrudFileSystemManager manager;
  EXPECT_EQ(manager.CreateDir(boost::filesystem::current_path()), true);
}

TEST(CrudFileSystemManager, TestCreateNewDir) {
  CrudFileSystemManager manager;
  boost::filesystem::path path = boost::filesystem::current_path() / "test";
  EXPECT_EQ(manager.CreateDir(path), true);
  boost::filesystem::remove(path);
}

TEST(CrudFileSystemManager, TestCreateFileAlreadyExist) {
  CrudFileSystemManager manager;
  EXPECT_EQ(manager.CreateFile(boost::filesystem::current_path(), "placeholder content"), false);
}

TEST(CrudFileSystemManager, ValidCreateNewFile) {
  CrudFileSystemManager manager;
  boost::filesystem::path path = boost::filesystem::current_path() / "test";
  EXPECT_EQ(manager.CreateFile(path, "placeholder content"), true);
  boost::filesystem::remove(path);
}

TEST(CrudFileSystemManager, TestReadFileBadFile) {
  CrudFileSystemManager manager;
  boost::beast::http::response<boost::beast::http::dynamic_body> res;
  EXPECT_EQ(manager.ReadFile(boost::filesystem::current_path() / "file_that_does_not_exist", res), false);
}

TEST(CrudFileSystemManager, TestValidReadFile) {
  CrudFileSystemManager manager;

  boost::filesystem::path path = boost::filesystem::current_path() / "test";
  EXPECT_EQ(manager.CreateFile(path, "placeholder content"), true);
  boost::beast::http::response<boost::beast::http::dynamic_body> res;
  EXPECT_EQ(manager.ReadFile(path, res), true);

  boost::filesystem::remove(path);
}

TEST(CrudFileSystemManager, TestWriteFileBadFile) {
  CrudFileSystemManager manager;
  EXPECT_EQ(manager.WriteFile(boost::filesystem::current_path() / "file_that_does_not_exist", "placeholder content"),
            false);
}

TEST(CrudFileSystemManager, TestValidWrite) {
  CrudFileSystemManager manager;
  boost::filesystem::path path = boost::filesystem::current_path() / "test";
  EXPECT_EQ(manager.CreateFile(path, "placeholder content"), true);
  EXPECT_EQ(manager.WriteFile(path, "placeholder content"), true);
  boost::filesystem::remove(path);
}

TEST(CrudFileSystemManager, TestDeleteFileBadFile) {
  CrudFileSystemManager manager;
  EXPECT_EQ(manager.DeleteFile(boost::filesystem::current_path() / "file_that_does_not_exist"), false);
}

TEST(CrudFileSystemManager, TestDeleteValidFile) {
  CrudFileSystemManager manager;
  boost::filesystem::path path = boost::filesystem::current_path() / "test";
  EXPECT_EQ(manager.CreateFile(path, "placeholder content"), true);
  EXPECT_EQ(manager.DeleteFile(path), true);
}

TEST(CrudFileSystemManager, TestDeleteLastFileInDir) {
  CrudFileSystemManager manager;
  boost::filesystem::path dir = boost::filesystem::current_path() / "test";
  boost::filesystem::create_directory(dir);
  boost::filesystem::path path = dir / "test";
  EXPECT_EQ(manager.CreateFile(path, "placeholder content"), true);
  EXPECT_EQ(manager.DeleteFile(path), true);
  EXPECT_EQ(boost::filesystem::exists(path), false);
  EXPECT_EQ(boost::filesystem::exists(dir), false);
}

TEST(CrudFileSystemManager, TestListFileNotDirectory) {
  CrudFileSystemManager manager;
  std::vector<std::string> res;
  EXPECT_EQ(manager.ListFiles(boost::filesystem::current_path() / "static_test_files/sample.txt", res), false);
  EXPECT_EQ(res.size(), 0);
}

TEST(CrudFileSystemManager, TestListFileIsDirectory) {
  CrudFileSystemManager manager;
  std::vector<std::string> res;
  EXPECT_EQ(manager.ListFiles(boost::filesystem::current_path() / "static_test_files", res), true);
  EXPECT_EQ(res.size(), 8);
}