#include "static_handler.h"

#include "gtest/gtest.h"

TEST(StaticHandler, StaticTest403) {
  string req_data = "Irrelevant stuff in request body";
  boost::beast::http::request<boost::beast::http::string_body> req{boost::beast::http::verb::get,  // GET
                                                                   "/static/sample.txt",           // URI
                                                                   11};                            // HTTP 1.1
  req.body() = req_data;
  boost::beast::http::response<boost::beast::http::dynamic_body> res;
  std::string root = "/usr/bin";
  StaticHandler handler(root, "sample_ip");
  handler.SetHeaders(req, res);
  EXPECT_EQ(res.version(), 11);
  EXPECT_EQ(res.result_int(), FORBIDDEN);
}

TEST(StaticHandler, StaticTest404) {
  string req_data = "Irrelevant stuff in request body";
  boost::beast::http::request<boost::beast::http::string_body> req{boost::beast::http::verb::get,                 // GET
                                                                   "/static/file_does_not_exist.fake-extension",  // URI
                                                                   11};  // HTTP 1.1
  boost::beast::http::response<boost::beast::http::dynamic_body> res;
  std::string root = "/usr/src/projects/";
  StaticHandler handler(root, "sample_ip");
  handler.SetHeaders(req, res);
  EXPECT_EQ(res.version(), 11);
  EXPECT_EQ(res.result_int(), NOT_FOUND);
}

TEST(StaticHandler, StaticTestTxtFile) {
  string req_data = "Irrelevant stuff in request body";
  boost::beast::http::request<boost::beast::http::string_body> req{boost::beast::http::verb::get,  // GET
                                                                   "/static/sample.txt",           // URI
                                                                   11};                            // HTTP 1.1
  req.body() = req_data;
  boost::beast::http::response<boost::beast::http::dynamic_body> res;
  std::string root = "/usr/src/projects/ctrl-c-ctrl-v/tests/static_test_files/sample.txt";
  StaticHandler handler(root, "sample_ip");
  handler.SetHeaders(req, res);
  auto it = res.find(boost::beast::http::field::content_type);
  EXPECT_NE(it, res.end());
  EXPECT_EQ(it->value().to_string(), "text/plain");
  EXPECT_EQ(res.version(), 11);
  EXPECT_EQ(res.result_int(), OK);
}

TEST(StaticHandler, StaticTestPngFile) {
  string req_data = "Irrelevant stuff in request body";
  boost::beast::http::request<boost::beast::http::string_body> req{boost::beast::http::verb::get,  // GET
                                                                   "/static/sample.png",           // URI
                                                                   11};                            // HTTP 1.1
  req.body() = req_data;
  boost::beast::http::response<boost::beast::http::dynamic_body> res;
  std::string root = "/usr/src/projects/ctrl-c-ctrl-v/tests/static_test_files/sample.png";
  StaticHandler handler(root, "sample_ip");
  handler.SetHeaders(req, res);
  auto it = res.find(boost::beast::http::field::content_type);
  EXPECT_NE(it, res.end());
  EXPECT_EQ(it->value().to_string(), "image/png");
  EXPECT_EQ(res.version(), 11);
  EXPECT_EQ(res.result_int(), OK);
}