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
  Session* sesh;
  boost::system::error_code ec;

  void SetUp() override {
    port = 80;

    sesh = new Session(io_service, serving_config);
  }

  void TearDown() override {}
};

// test returning socket
TEST_F(SessionTest, TestSocket) { ASSERT_FALSE(sesh->get_socket().is_open()); }
