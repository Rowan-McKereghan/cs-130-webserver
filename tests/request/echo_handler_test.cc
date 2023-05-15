#include "echo_handler.h"

#include "gtest/gtest.h"
#include "request.h"
#include "response.h"

TEST(EchoHandler, EchoTest) {
  boost::asio::ip::tcp::socket* socket;
  std::string req_data = "Sample data to be echoed";
  Request req(req_data);
  req.method_ = "GET";
  req.uri_ = "/echo";
  req.http_version_ = 1;

  Response res(socket);
  EchoHandler handler;
  handler.HandleRequest(req, res);

  EXPECT_EQ(res.get_data(), req.raw_request_);
  EXPECT_EQ(res.get_headers().size(), 2);
  EXPECT_EQ(res.get_status_code(), OK);
}