#include "static_handler.h"

#include "gtest/gtest.h"
#include "response.h"

TEST(StaticHandler, StaticTest403) {
  boost::asio::ip::tcp::socket* socket;
  string req_data = "Irrelevant stuff in request body";
  Request req(req_data);
  req.method_ = "GET";
  req.uri_ = "/static/sample.txt";
  req.http_version_ = 1;

  Response res(socket);
  std::string root = "/usr/bin";
  StaticHandler handler(root);
  handler.SetHeaders(req, res);
  EXPECT_EQ(res.get_headers().size(), 2);
  EXPECT_EQ(res.get_status_code(), FORBIDDEN);
}

TEST(StaticHandler, StaticTest404) {
  boost::asio::ip::tcp::socket* socket;
  string req_data = "Irrelevant stuff in request body";
  Request req(req_data);
  req.method_ = "GET";
  req.uri_ = "/static/file_does_not_exist.fake-extension";
  req.http_version_ = 1;

  Response res(socket);
  std::string root = "/usr/src/projects/";
  StaticHandler handler(root);
  handler.SetHeaders(req, res);
  EXPECT_EQ(res.get_headers().size(), 2);
  EXPECT_EQ(res.get_status_code(), NOT_FOUND);
}

TEST(StaticHandler, StaticTestTxtFile) {
  boost::asio::ip::tcp::socket* socket;
  string req_data = "Irrelevant stuff in request body";
  Request req(req_data);
  req.method_ = "GET";
  req.uri_ = "/static/sample.txt";
  req.http_version_ = 1;

  Response res(socket);
  std::string root =
      "/usr/src/projects/ctrl-c-ctrl-v/tests/static_test_files/sample.txt";
  StaticHandler handler(root);
  handler.SetHeaders(req, res);
  EXPECT_EQ(res.get_headers().size(), 2);
  EXPECT_EQ(res.get_status_code(), OK);
  EXPECT_EQ(res.get_headers()[0].value, "text/plain");
}

TEST(StaticHandler, StaticTestPngFile) {
  boost::asio::ip::tcp::socket* socket;
  string req_data = "Irrelevant stuff in request body";
  Request req(req_data);
  req.method_ = "GET";
  req.uri_ = "/static/sample.png";
  req.http_version_ = 1;

  Response res(socket);
  std::string root =
      "/usr/src/projects/ctrl-c-ctrl-v/tests/static_test_files/sample.png";
  StaticHandler handler(root);
  handler.SetHeaders(req, res);
  EXPECT_EQ(res.get_headers().size(), 2);
  EXPECT_EQ(res.get_status_code(), OK);
  EXPECT_EQ(res.get_headers()[0].value, "image/png");
}