#include "static_handler_factory.h"

#include <string>

#include "config_parser.h"
#include "gtest/gtest.h"

TEST(StaticHandlerFactory, TestGetRoot) {
  NginxConfig config;
  std::shared_ptr<NginxConfigStatement> statement =
      std::make_shared<NginxConfigStatement>();
  statement->tokens_ = {"root", "tests/static_test_files"};
  config.statements_ = {statement};
  StaticHandlerFactory static_handler_factory(&config);
  EXPECT_EQ(static_handler_factory.GetRoot(), "tests/static_test_files/");
  std::cout << "still no segfault here" << std::endl;
}

TEST(StaticHandlerFactory, TestGetCompleteFilePath) {
  NginxConfig config;
  std::shared_ptr<NginxConfigStatement> statement =
      std::make_shared<NginxConfigStatement>();
  statement->tokens_ = {"root", "tests/static_test_files"};
  config.statements_ = {statement};
  StaticHandlerFactory static_handler_factory(&config);
  std::string file_path = "index.html";
  EXPECT_EQ(static_handler_factory.GetCompleteFilePath(file_path),
            "tests/static_test_files/index.html");
}