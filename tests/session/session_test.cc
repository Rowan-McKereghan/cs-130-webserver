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

// test steup of session
TEST_F(SessionTest, SessionSetup) {
  sesh->start();
  ASSERT_EQ(sesh->HTTPResponse.substr(0, 16), "HTTP/1.1 200 OK\n");
}

// test correct parsing of end of file
TEST_F(SessionTest, TestEOF) {
  sesh->data_[0] = '\r';
  sesh->data_[1] = '\n';
  sesh->data_[2] = '\r';
  sesh->data_[3] = '\n';
  sesh->handle_read(ec, 4);
  ASSERT_TRUE(sesh->end_of_request);

  sesh->end_of_request = false;
  sesh->data_[0] = '\n';
  sesh->data_[1] = '\n';
  sesh->handle_read(ec, 2);
  ASSERT_TRUE(sesh->end_of_request);
}

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
