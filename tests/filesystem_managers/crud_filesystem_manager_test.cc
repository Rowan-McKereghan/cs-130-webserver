#include "crud_filesystem_manager.h"

#include "gtest/gtest.h"

// Tests for the operations that don't involve creating/deleting/writing to files
// Testing file operations will be done in integration tests
TEST(CrudFileSystemManager, TestCreateDirAlreadyExist) {
  CrudFileSystemManager manager;
  EXPECT_EQ(manager.CreateDir(boost::filesystem::current_path()), true);
}

TEST(CrudFileSystemManager, TestCreateFileAlreadyExist) {
  CrudFileSystemManager manager;
  EXPECT_EQ(manager.CreateFile(boost::filesystem::current_path(), "placeholder content"), false);
}

TEST(CrudFileSystemManager, TestReadFileBadFile) {
  CrudFileSystemManager manager;
  boost::beast::http::response<boost::beast::http::dynamic_body> res;
  EXPECT_EQ(manager.ReadFile(boost::filesystem::current_path() / "file_that_does_not_exist", res), false);
}

TEST(CrudFileSystemManager, TestWriteFileBadFile) {
  CrudFileSystemManager manager;
  EXPECT_EQ(manager.WriteFile(boost::filesystem::current_path() / "file_that_does_not_exist", "placeholder content"),
            false);
}

TEST(CrudFileSystemManager, TestDeleteFileBadFile) {
  CrudFileSystemManager manager;
  EXPECT_EQ(manager.DeleteFile(boost::filesystem::current_path() / "file_that_does_not_exist"), false);
}

TEST(CrudFileSystemManager, TestListFileNotDirectory) {
  CrudFileSystemManager manager;
  EXPECT_EQ(manager.ListFiles(boost::filesystem::current_path() / "static_test_files/sample.txt").size(), 0);
}

TEST(CrudFileSystemManager, TestListFileIsDirectory) {
  CrudFileSystemManager manager;
  EXPECT_EQ(manager.ListFiles(boost::filesystem::current_path() / "static_test_files").size(), 6);
}
