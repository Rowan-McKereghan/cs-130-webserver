#include "request.h"

#include "gtest/gtest.h"
#include "echo_handler.h"
#include "request_dispatcher.h"

// Test the Request::ParseHTTPRequest() method
TEST(RequestParserTest, ValidEchoRequest) {
  std::string req_str =
      "GET /echo HTTP/1.1\r\n"
      "Host: localhost\r\n"
      "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:108.0) "
      "Gecko/20100101 Firefox/108.0\r\n"
      "Accept: "
      "text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,/"
      ";q=0.8\r\n"
      "Accept-Language: en-US,en;q=0.5\r\n"
      "Accept-Encoding: gzip, deflate\r\n"
      "Connection: keep-alive\r\n"
      "\r\n";
  Request req(req_str);
  EXPECT_EQ(req.method_, "GET");
  EXPECT_EQ(req.uri_, "/echo");
  EXPECT_EQ(req.http_version_, 1);
  EXPECT_EQ(req.headers_.size(), 6);
}

// Test Response class with different status codes
TEST(ResponseTest, ResponseWithStatusCodes) {
  boost::asio::io_context io_context;
  boost::asio::ip::tcp::socket socket(io_context);

  // Test with status code OK (200)
  Response res(&socket);
  res.set_status_code(OK);
  std::string response_str = res.GenerateHTTPResponse();
  EXPECT_TRUE(response_str.find("HTTP/1.1 200 OK") != std::string::npos);

  // Test with status code BAD_REQUEST (400)
  res.set_status_code(BAD_REQUEST);
  response_str = res.GenerateHTTPResponse();
  EXPECT_TRUE(response_str.find("HTTP/1.1 400 Bad Request") !=
              std::string::npos);

  // Test with status code NOT_FOUND (404)
  res.set_status_code(NOT_FOUND);
  response_str = res.GenerateHTTPResponse();
  EXPECT_TRUE(response_str.find("HTTP/1.1 404 Not Found") != std::string::npos);

  // Test with status code INTERNAL_SERVER_ERROR (500)
  res.set_status_code(INTERNAL_SERVER_ERROR);
  response_str = res.GenerateHTTPResponse();
  EXPECT_TRUE(response_str.find("HTTP/1.1 500 Internal Server Error") !=
              std::string::npos);
}

TEST(RequestParserTest, InvalidRequestType) {
  boost::asio::io_context io_context;
  boost::asio::ip::tcp::socket socket(io_context);
  std::string req_str =
      "GET /invalid_uri HTTP/1.1\r\n"
      "Host: localhost\r\n"
      "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:108.0) "
      "Gecko/20100101 Firefox/108.0\r\n"
      "Accept: "
      "text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,"
      "*;q=0.8\r\n"
      "Accept-Language: en-US,en;q=0.5\r\n"
      "Accept-Encoding: gzip, deflate\n"
      "Connection: keep-alive\r\n"
      "\r\n";
  Request req(req_str);
  Response res(&socket);
  RequestDispatcher req_dispatcher;
  ServingConfig serving_config{
      {{"/static1", "/usr/src/projects/"}},  // static_file_paths
      {"/echo2"}                             // echo_paths
  };
  std::string client_ip = "127.0.0.1";
  req_dispatcher.RouteRequest(req, res, serving_config, client_ip);
  EXPECT_EQ(res.get_status_code(), BAD_REQUEST);
  EXPECT_EQ(res.get_headers().size(), 2);
}