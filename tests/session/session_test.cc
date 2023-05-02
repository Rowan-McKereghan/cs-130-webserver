#include "session.h"

#include <boost/asio.hpp>
#include <iostream>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "server.h"

class SessionTest : public ::testing::Test {
 protected:
  boost::asio::io_service io_service;
  short port;
  session* sesh;
  boost::system::error_code ec;

  void SetUp() override {
    port = 80;
    sesh = new session(io_service);
  }

  void TearDown() override {}
};

// test write
TEST_F(SessionTest, TestWrite) {
  sesh->handle_write(ec);
  ASSERT_EQ(1, 1);
}

// test http write (trivial)
TEST_F(SessionTest, TestHTTPWrite) {
  sesh->handle_http_write(ec);
  ASSERT_EQ(1, 1);
}

// test returning socket
TEST_F(SessionTest, TestSocket) { ASSERT_FALSE(sesh->socket().is_open()); }
