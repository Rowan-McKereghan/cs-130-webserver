#include "session.h"

#include <boost/asio.hpp>
#include <iostream>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "server.h"

class MockSession : public session {
 public:
  MockSession(boost::asio::io_service& io_service) : session(io_service) {}
  MOCK_METHOD(void, format_http_response, (), (override));
  MOCK_METHOD(void, check_if_http_request_ends, (size_t bytes_transferred),
              (override));
};

class SessionTest : public ::testing::Test {
 protected:
  boost::asio::io_service io_service;
  short port;
  MockSession* session;
  boost::system::error_code ec;

  void SetUp() override {
    port = 80;
    session = new MockSession(io_service);
  }

  void TearDown() override { delete session; }
};

TEST_F(SessionTest, SessionSetup) {
  boost::asio::ip::tcp::socket sock = boost::asio::ip::tcp::socket(io_service);
  testing::DefaultValue<boost::asio::ip::tcp::socket&>::Set(sock);

  EXPECT_CALL(*session, format_http_response()).Times(1);
  EXPECT_CALL(*session, check_if_http_request_ends(4)).Times(1);

  session->data_[0] = '\r';
  session->data_[1] = '\n';
  session->data_[2] = '\r';
  session->data_[3] = '\n';
  session->start();
  session->handle_read(ec, 4);
}
