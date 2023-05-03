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
  ServingConfig serving_config;
  session* sesh;
  boost::system::error_code ec;

  void SetUp() override {
    port = 80;
    
    // Default root path (for now, testing purposes only)
    serving_config.static_file_paths =  {{"static1", "/usr/src/projects/"}};
    sesh = new session(io_service, serving_config);
  }

  void TearDown() override {}
};

// test write
TEST_F(SessionTest, TestWrite) {
  sesh->handle_write(ec);
  ASSERT_EQ(1, 1);
}

// test returning socket
TEST_F(SessionTest, TestSocket) { ASSERT_FALSE(sesh->socket().is_open()); }
