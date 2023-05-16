#include "echo_handler.h"

#include "gtest/gtest.h"

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>

TEST(EchoHandlerTest, EchoTest) {
  std::string req_data = "Sample data to be echoed";
  boost::beast::http::request<boost::beast::http::string_body> req {
        boost::beast::http::verb::get, //GET
        "/echo", //URI
        11}; //HTTP 1.1
  req.body() = req_data;
  boost::beast::http::response<boost::beast::http::dynamic_body> res;
  EchoHandler handler;
  handler.HandleRequest(req, res);

  boost::beast::http::dynamic_body::value_type req_body;
  boost::beast::ostream(req_body) << req;

  EXPECT_EQ(boost::beast::buffers_to_string(res.body().data()), boost::beast::buffers_to_string(req_body.data()));
  EXPECT_EQ(res.version(), 11);
  EXPECT_EQ(res.result_int(), OK);
}