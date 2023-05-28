#include "session.h"

#include <boost/asio.hpp>
#include <boost/beast/http.hpp>
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
  Session* session;
  boost::system::error_code ec;

  void SetUp() override {
    port = 80;

    session = new Session(io_service, serving_config);
  }

  void TearDown() override {}
};

// test returning socket
TEST_F(SessionTest, TestSocket) { ASSERT_FALSE(session->get_socket().is_open()); }

// test routing a request with no error
TEST_F(SessionTest, TestRouteSuccess) {
  session->req = std::make_shared<boost::beast::http::request<boost::beast::http::string_body>>();
  session->PrepareResponse(ec, 0, "127.0.0.1");
  auto dateField = session->res.find(boost::beast::http::field::date);

  // date field is only set when error parameter is not set
  ASSERT_NE(dateField, session->res.end());
}

// testing handling request with bad target URI error
TEST_F(SessionTest, TestRouteBadTarget) {
  session->req = std::make_shared<boost::beast::http::request<boost::beast::http::string_body>>();
  session->PrepareResponse(boost::beast::http::error::bad_target, 0, "127.0.0.1");
  ASSERT_EQ(session->res.result(), boost::beast::http::status::bad_request);
}

// testing handling request with other error
TEST_F(SessionTest, TestRouteOtherError) {
  session->req = std::make_shared<boost::beast::http::request<boost::beast::http::string_body>>();
  session->PrepareResponse(boost::beast::http::error::bad_value, 0, "127.0.0.1");
  auto dateField = session->res.find(boost::beast::http::field::date);

  // date field is not set in this branch
  ASSERT_EQ(dateField, session->res.end());
}
