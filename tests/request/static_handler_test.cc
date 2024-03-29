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
  StaticHandler handler(root);
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
  std::string root = ".";
  StaticHandler handler(root);
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
  std::string root = "./static_test_files/sample.txt";
  StaticHandler handler(root);
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
  std::string root = "./static_test_files/sample.png";
  StaticHandler handler(root);
  handler.SetHeaders(req, res);
  auto it = res.find(boost::beast::http::field::content_type);
  EXPECT_NE(it, res.end());
  EXPECT_EQ(it->value().to_string(), "image/png");
  EXPECT_EQ(res.version(), 11);
  EXPECT_EQ(res.result_int(), OK);

}

TEST(StaticHandler, StaticTestHTMLFile) {
  string req_data = "Irrelevant stuff in request body";
  boost::beast::http::request<boost::beast::http::string_body> req{boost::beast::http::verb::get,  // GET
                                                                   "/static/sample.html",           // URI
                                                                   11};                            // HTTP 1.1
  req.body() = req_data;
  boost::beast::http::response<boost::beast::http::dynamic_body> res;
  std::string root = "./static_test_files/sample.html";
  StaticHandler handler(root);
  handler.SetHeaders(req, res);
  auto it = res.find(boost::beast::http::field::content_type);
  EXPECT_NE(it, res.end());
  EXPECT_EQ(it->value().to_string(), "text/html");
  EXPECT_EQ(res.version(), 11);
  EXPECT_EQ(res.result_int(), OK);
}

TEST(StaticHandler, StaticTestZipFile) {
  string req_data = "Irrelevant stuff in request body";
  boost::beast::http::request<boost::beast::http::string_body> req{boost::beast::http::verb::get,  // GET
                                                                   "/static/sample.zip",           // URI
                                                                   11};                            // HTTP 1.1
  req.body() = req_data;
  boost::beast::http::response<boost::beast::http::dynamic_body> res;
  std::string root = "./static_test_files/sample.png";
  StaticHandler handler(root);
  handler.SetHeaders(req, res);
  auto it = res.find(boost::beast::http::field::content_type);
  EXPECT_NE(it, res.end());
  EXPECT_EQ(it->value().to_string(), "image/png");
  EXPECT_EQ(res.version(), 11);
  EXPECT_EQ(res.result_int(), OK);
}

TEST(StaticHandler, StaticTestPDFFile) {
  string req_data = "Irrelevant stuff in request body";
  boost::beast::http::request<boost::beast::http::string_body> req{boost::beast::http::verb::get,  // GET
                                                                   "/static/sample.pdf",           // URI
                                                                   11};                            // HTTP 1.1
  req.body() = req_data;
  boost::beast::http::response<boost::beast::http::dynamic_body> res;
  std::string root = "./static_test_files/sample.pdf";
  StaticHandler handler(root);
  handler.SetHeaders(req, res);
  auto it = res.find(boost::beast::http::field::content_type);
  EXPECT_NE(it, res.end());
  EXPECT_EQ(it->value().to_string(), "application/pdf");
  EXPECT_EQ(res.version(), 11);
  EXPECT_EQ(res.result_int(), OK);
}

TEST(StaticHandler, TestStaticHandlerHandleRequestTxtFile) {
  string req_data = "Irrelevant stuff in request body";
  boost::beast::http::request<boost::beast::http::string_body> req{boost::beast::http::verb::get,  // GET
                                                                   "/static/sample.txt",           // URI
                                                                   11};                            // HTTP 1.1
  req.body() = req_data;
  boost::beast::http::response<boost::beast::http::dynamic_body> res;
  std::string root = "./static_test_files/sample.txt";
  StaticHandler handler(root);
  handler.HandleRequest(req, res);
  EXPECT_EQ(res.result_int(), OK);
  auto it = res.find(boost::beast::http::field::content_type);
  EXPECT_EQ(it->value().to_string(), "text/plain");
  EXPECT_EQ(res.version(), 11);
}

TEST(StaticHandler, TestHandleRequestInvalidPath) {
  string req_data = "Irrelevant stuff in request body";
  boost::beast::http::request<boost::beast::http::string_body> req{boost::beast::http::verb::get,  // GET
                                                                   "/static/sample.txt",           // URI
                                                                   11};                            // HTTP 1.1
  req.body() = req_data;
  boost::beast::http::response<boost::beast::http::dynamic_body> res;
  std::string root = "./static_test_files/invalid.txt";
  StaticHandler handler(root);
  handler.HandleRequest(req, res);
  EXPECT_EQ(res.result_int(), NOT_FOUND);
}
