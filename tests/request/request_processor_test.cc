#include "request_processor.h"
#include "gtest/gtest.h"

#include <boost/asio.hpp>
#include <boost/filesystem.hpp>
#include "request_handler_echo.h"



// Test the Request::ParseHTTPRequest() method
TEST(RequestParserTest, ValidEchoRequest) {
  std::string req =
      "GET /echo HTTP/1.1\r\n"
      "Host: localhost\r\n"
      "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:108.0) "
      "Gecko/20100101 Firefox/108.0\r\n"
      "Accept: "
      "text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/"
      "*;q=0.8\r\n"
      "Accept-Language: en-US,en;q=0.5\r\n"
      "Accept-Encoding: gzip, deflate\r\n"
      "Connection: keep-alive\r\n"
      "\r\n";
  Request req_obj = Request::ParseHTTPRequest(req);
  EXPECT_EQ(req_obj.method, "GET");
  EXPECT_EQ(req_obj.uri, "/echo");
  EXPECT_EQ(req_obj.http_version, 1);
  EXPECT_EQ(req_obj.headers.size(), 6);
  EXPECT_EQ(req_obj.uri_base_path, "echo");
}

TEST(RequestParserTest, ValidStaticRequestSingleForwardSlash) {
  std::string req =
      "GET /static/images HTTP/1.1\r\n"
      "Host:    localhost\r\n"
      "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:108.0) "
      "Gecko/20100101 Firefox/108.0\r\n"
      "Accept: "
      "text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,"
      "*;q=0.8\r\n"
      "Accept-Language: en-US,en;q=0.5\r\n"
      "Accept-Encoding: gzip, deflate\n"  // RFC7230, § 3.5 recomends allowing
                                          // just a \n to terminate while
                                          // parsing
      "Connection: keep-alive\r\n"
      "\r\n";
  Request req_obj = Request::ParseHTTPRequest(req);
  EXPECT_EQ(req_obj.method, "GET");
  EXPECT_EQ(req_obj.uri, "/static/images");
  EXPECT_EQ(req_obj.http_version, 1);
  EXPECT_EQ(req_obj.headers.size(), 6);
  EXPECT_EQ(req_obj.uri_base_path, "static");
}

TEST(RequestParserTest,
     ValidStaticRequestMultipleForwardSlashesAndFileExtension) {
  std::string req =
      "GET /static/images/nest1/list.jpg HTTP/1.1\r\n"
      "Host:    localhost\r\n"
      "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:108.0) "
      "Gecko/20100101 Firefox/108.0\r\n"
      "Accept: "
      "text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,"
      "*;q=0.8\r\n"
      "Accept-Language: en-US,en;q=0.5\r\n"
      "Accept-Encoding: gzip, deflate\n"  // RFC7230, § 3.5 recomends allowing
                                          // just a \n to terminate while
                                          // parsing
      "Connection: keep-alive\r\n"
      "\r\n";
  Request req_obj = Request::ParseHTTPRequest(req);
  EXPECT_EQ(req_obj.method, "GET");
  EXPECT_EQ(req_obj.uri, "/static/images/nest1/list.jpg");
  EXPECT_EQ(req_obj.http_version, 1);
  EXPECT_EQ(req_obj.headers.size(), 6);
  EXPECT_EQ(req_obj.uri_base_path, "static");
}

TEST(RequestParserTest, InvalidRequestType) {
  boost::asio::ip::tcp::socket* socket;
  std::string req =
      "GET /invalid_uri HTTP/1.1\r\n"
      "Host:    localhost\r\n"
      "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:108.0) "
      "Gecko/20100101 Firefox/108.0\r\n"
      "Accept: "
      "text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,"
      "*;q=0.8\r\n"
      "Accept-Language: en-US,en;q=0.5\r\n"
      "Accept-Encoding: gzip, deflate\n"  // RFC7230, § 3.5 recomends allowing
                                          // just a \n to terminate while
                                          // parsing
      "Connection: keep-alive\r\n"
      "\r\n";
  Request req_obj = Request::ParseHTTPRequest(req);
  Response res(socket);
  RequestProcessor req_processor;
  boost::filesystem::path root1{"/usr/src/projects/"};
  req_processor.RouteRequest(req, root1, res);
  EXPECT_EQ(res.status_code, NOT_FOUND);
  EXPECT_EQ(res.headers.size(), 2);
  EXPECT_EQ(res.data, "404 Not Found\r\n\r\n");
}

TEST(RequestProcessorTest, RouteRequest) {
  boost::asio::ip::tcp::socket* socket;
  std::string req =
      "GET /echo HTTP/1.1\r\n"
      "Host: localhost\r\n"
      "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:108.0) "
      "Gecko/20100101 Firefox/108.0\r\n"
      "Accept: "
      "text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/"
      "*;q=0.8\r\n"
      "Accept-Language: en-US,en;q=0.5\r\n"
      "Accept-Encoding: gzip, deflate\r\n"
      "Connection: keep-alive\r\n"
      "\r\n";
  Response res(socket);
  RequestProcessor req_processor;
  boost::filesystem::path root1{"/usr/src/projects/"};
  req_processor.RouteRequest(req, root1, res);
  EXPECT_EQ(res.status_code, OK);
  EXPECT_EQ(res.headers.size(), 2);
  EXPECT_EQ(res.data, req);
}