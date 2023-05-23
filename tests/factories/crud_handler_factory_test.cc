#include "crud_handler_factory.h"

#include <string>

#include "config_parser.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "static_handler_factory.h"

// Test crud handler factory
TEST(HandlerFactoryTest, TestStaticHandlerFactory) {
  NginxConfig config;
  CrudHandlerFactory crudHandlerFactory(&config);

  // checks if the objects are created of the right types
  std::string temp = "";
  I_RequestHandler *handler = crudHandlerFactory.CreateHandler(temp);
  EXPECT_TRUE(dynamic_cast<StaticHandler *>(handler) == nullptr);

  EXPECT_FALSE(dynamic_cast<CrudHandler *>(handler) == nullptr);
}
