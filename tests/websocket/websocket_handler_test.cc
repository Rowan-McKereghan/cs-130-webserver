#include "websocket_handler.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <boost/asio.hpp>
#include <boost/beast/http.hpp>

#include "global_websocket_state.h"
#include "logger.h"

using namespace ::testing;
using namespace boost::asio;

class MockSocket : public ip::tcp::socket {
 public:
  MockSocket(io_service &service) : ip::tcp::socket(service) {}
};

class MockGlobalWebsocketState : public GlobalWebsocketState {
 public:
  MOCK_METHOD(void, Add, (const std::string &, WebsocketHandler *), (override));
  MOCK_METHOD(void, Remove, (const std::string &, WebsocketHandler *), (override));
  MOCK_METHOD(void, Broadcast, (const std::string &, std::string), (override));
};

class WebsocketHandlerTest : public ::testing::Test {
 protected:
  io_service io_service_;
  MockSocket mock_socket_;
  std::shared_ptr<MockGlobalWebsocketState> mock_state_;
  io_service::strand strand_;

  WebsocketHandlerTest()
      : mock_socket_(io_service_), strand_(io_service_), mock_state_(std::make_shared<MockGlobalWebsocketState>()) {}
};

// TODO: Expect call to Add
TEST_F(WebsocketHandlerTest, ConstructorDestructorTest) {
  EXPECT_CALL(*mock_state_, Remove(_, _)).Times(1);
  auto websocket_handler = std::make_shared<WebsocketHandler>(strand_, mock_socket_, mock_state_);
  boost::beast::http::request<boost::beast::http::string_body> req;
  ASSERT_TRUE(websocket_handler != nullptr);
}