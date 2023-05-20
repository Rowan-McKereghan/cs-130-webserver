#include "I_file_handler_factory.h"

#include <string>

#include "config_parser.h"
#include "gtest/gtest.h"

TEST(I_FileHandlerFactory, TestGetRoot) {
  NginxConfig config;
  std::shared_ptr<NginxConfigStatement> statement = std::make_shared<NginxConfigStatement>();
  statement->tokens_ = {"root", "tests/file_test_files"};
  config.statements_ = {statement};
  I_FileHandlerFactory* file_handler_factory;
  EXPECT_EQ(file_handler_factory->GetRoot(&config), "tests/file_test_files/");
}

TEST(I_FileHandlerFactory, TestGetCompleteFilePath) {
  NginxConfig config;
  std::shared_ptr<NginxConfigStatement> statement = std::make_shared<NginxConfigStatement>();
  statement->tokens_ = {"root", "tests/file_test_files"};
  config.statements_ = {statement};
  I_FileHandlerFactory* file_handler_factory;
  std::string file_path = "index.html";
  EXPECT_EQ(file_handler_factory->GetCompleteFilePath(&config, file_path), "tests/file_test_files/index.html");
}