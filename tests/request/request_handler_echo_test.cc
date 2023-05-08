#include "request_handler_echo.h"

#include "gtest/gtest.h"
#include "request.h"
#include "response.h"

TEST(RequestHandlerEcho, EchoTest) {
  boost::asio::ip::tcp::socket* socket;
  std::string req_data = "Sample data to be echoed";
  Request req(req_data);
  req.method = "GET";
  req.uri = "/echo";
  req.http_version = 1;

  Response res(socket);
  RequestHandlerEcho handler;
  handler.HandleRequest(req, res);

  EXPECT_EQ(res.get_data(), req.raw_request);
  EXPECT_EQ(res.get_headers().size(), 2);
  EXPECT_EQ(res.get_status_code(), OK);
}