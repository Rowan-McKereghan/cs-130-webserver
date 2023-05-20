#include "crud_handler_factory.h"

#include <string>

#include "config_parser.h"
#include "gtest/gtest.h"

// Same as static handler factory tests because these methods are the same between the classes

TEST(CrudHandlerFactory, TestGetRoot) {
  NginxConfig config;
  std::shared_ptr<NginxConfigStatement> statement = std::make_shared<NginxConfigStatement>();
  statement->tokens_ = {"root", "tests/crud_test_files"};
  config.statements_ = {statement};
  CrudHandlerFactory crud_handler_factory(&config);
  EXPECT_EQ(crud_handler_factory.GetRoot(), "tests/crud_test_files/");
}

TEST(CrudHandlerFactory, TestGetCompleteFilePath) {
  NginxConfig config;
  std::shared_ptr<NginxConfigStatement> statement = std::make_shared<NginxConfigStatement>();
  statement->tokens_ = {"root", "tests/crud_test_files"};
  config.statements_ = {statement};
  CrudHandlerFactory crud_handler_factory(&config);
  std::string file_path = "index.html";
  EXPECT_EQ(crud_handler_factory.GetCompleteFilePath(file_path), "tests/crud_test_files/index.html");
}