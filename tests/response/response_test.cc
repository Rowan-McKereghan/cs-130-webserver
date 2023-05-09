// response_test.cc
#include "response.h"

#include <regex>
#include <string>

#include "gtest/gtest.h"

TEST(ResponseTest, GenerateHttpResponse) {
  boost::asio::ip::tcp::socket* socket;
  Response res(socket);
  res.set_status_code(OK);

  std::string body_str = "Hello world";
  res.set_body(body_str);
  res.add_header("Content-Type", "text/plain");
  res.add_header("Content-Length", std::to_string(body_str.length()));

  std::string expected_response =
      "HTTP/1.1 200 OK\r\n"
      "Date: .* GMT\r\n"
      "Server: webserver\r\n"
      "Content-Type: text/plain\r\n"
      "Content-Length: 11\r\n"
      "\r\n"
      "Hello world";

  std::string actual_response = res.GenerateHTTPResponse();

  // Use a regular expression to match the generated date
  std::regex expected_response_regex(expected_response, std::regex::extended);
  EXPECT_TRUE(std::regex_search(actual_response, expected_response_regex));
}