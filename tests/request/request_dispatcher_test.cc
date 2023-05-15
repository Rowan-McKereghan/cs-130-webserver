#include "request_dispatcher.h"

#include <boost/asio.hpp>
#include <boost/filesystem.hpp>

#include "gtest/gtest.h"
#include "request.h"
#include "echo_handler.h"
#include "response.h"

TEST(RequestDispatcherTest, RouteRequest) {
  boost::asio::ip::tcp::socket* socket;
  std::string req_str =
      "GET /echo2 HTTP/1.1\r\n"
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
  Request req(req_str);
  Response res(socket);
  RequestDispatcher req_dispatcher;
  ServingConfig serving_config{
      {{"/static1", "/usr/src/projects/"}},  // static_file_paths
      {"/echo2"}                             // echo_paths
  };
  req_dispatcher.RouteRequest(req, res, serving_config, "127.0.0.1");
  EXPECT_EQ(res.get_status_code(), OK);
  EXPECT_EQ(res.get_headers().size(), 2);
  EXPECT_EQ(res.get_data(), req_str);
}