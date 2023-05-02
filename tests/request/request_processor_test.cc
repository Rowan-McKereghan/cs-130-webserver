#include "request_processor.h"

#include "gtest/gtest.h"
// Test the Request::ParseHTTPRequest() method
TEST(RequestTest, ParseHTTPRequest) {
  std::string req1 =
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
  Request req_obj = Request::ParseHTTPRequest(req1);
  EXPECT_EQ(req_obj.method, "GET");
  EXPECT_EQ(req_obj.uri, "/echo");
  EXPECT_EQ(req_obj.http_version, 1);
  EXPECT_EQ(req_obj.headers.size(), 6);
  EXPECT_EQ(req_obj.uri_base_path, "echo");
  std::string req2 =
      "GET /static/images/logo.png HTTP/1.1\r\n"
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
  Request req_obj2 = Request::ParseHTTPRequest(req2);
  EXPECT_EQ(req_obj2.method, "GET");
  EXPECT_EQ(req_obj2.uri, "/static/images/logo.png");
  EXPECT_EQ(req_obj2.http_version, 1);
  EXPECT_EQ(req_obj2.headers.size(), 6);
  EXPECT_EQ(req_obj2.uri_base_path, "static");
}
// Test the RequestProcessor::ParseRequest() method
TEST(RequestProcessorTest, ParseRequest) {
  std::string req1 =
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
  Response res1;
  RequestProcessor req_processor_1;
  req_processor_1.ParseRequest(req1, res1);
  EXPECT_EQ(res1.status_code, OK);
  EXPECT_EQ(res1.headers.size(), 2);
  EXPECT_EQ(res1.data, req1);
  // TODO: Fix invalid path
  //   std::string req2 =
  //       "GET /invalid_uri HTTP/1.1\r\n"
  //       "Host: localhost\r\n"
  //       "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:108.0) "
  //       "Gecko/20100101 Firefox/108.0\r\n"
  //       "Accept: "
  //       "text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,/"
  //       ";q=0.8\r\n"
  //       "Accept-Language: en-US,en;q=0.5\r\n"
  //       "Accept-Encoding: gzip, deflate\r\n"
  //       "Connection: keep-alive\r\n"
  //       "\r\n";
  //   Response res2;
  //   RequestProcessor req_processor_2;
  //   req_processor_2.ParseRequest(req2, res2);
  //   EXPECT_EQ(res2.status_code, NOT_FOUND);
  //   EXPECT_EQ(res2.headers.size(), 6);
  //   EXPECT_EQ(res2.data, "404 Not Found\r\n\r\n");
  // TODO: Enable static images
  //   std::string req3 =
  //       "GET /static/images/logo.png HTTP/1.1\r\n"
  //       "Host: localhost\r\n"
  //       "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:108.0) "
  //       "Gecko/20100101 Firefox/108.0\r\n"
  //       "Accept: "
  //       "text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,/"
  //       ";q=0.8\r\n"
  //       "Accept-Language: en-US,en;q=0.5\r\n"
  //       "Accept-Encoding: gzip, deflate\r\n"
  //       "Connection: keep-alive\r\n"
  //       "\r\n";
  //   Response res3;
  //   RequestProcessor req_processor_3;
  //   req_processor_3.ParseRequest(req3, res3);
  //   EXPECT_EQ(res3.status_code, OK);
  //   EXPECT_EQ(res3.headers.size(), 6);
  //   EXPECT_EQ(res3.data, "");
}