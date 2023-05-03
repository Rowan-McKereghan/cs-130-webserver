#include "session.h"

#include <boost/asio.hpp>
#include <boost/filesystem.hpp>
#include <iostream>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "server.h"

class SessionTest : public ::testing::Test {
 protected:
  boost::asio::io_service io_service;
  short port;
  boost::filesystem::path root;
  session* sesh;
  boost::system::error_code ec;

  void SetUp() override {
    port = 80;
    root = {"/usr/src/projects/"}; //default root path (for now, testing purposes only)
    sesh = new session(io_service, root);
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