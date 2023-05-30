#include "not_found_handler.h"

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>

#include "gtest/gtest.h"

TEST(BasicNotFoundTest, NotFoundTest) {
  std::string req_data = "Ignored Body";
  boost::beast::http::request<boost::beast::http::string_body> req{boost::beast::http::verb::get,  // GET
                                                                   "/",                            // URI
                                                                   11};                            // HTTP 1.1
  req.body() = req_data;
  boost::beast::http::response<boost::beast::http::dynamic_body> res;
  NotFoundHandler handler("sample_ip");
  handler.HandleRequest(req, res);

  boost::beast::http::dynamic_body::value_type req_body;
  boost::beast::ostream(req_body) << req;

  EXPECT_EQ(boost::beast::buffers_to_string(res.body().data()), "404 Not Found");
  EXPECT_EQ(res.version(), 11);
  EXPECT_EQ(res.result_int(), NOT_FOUND);
}