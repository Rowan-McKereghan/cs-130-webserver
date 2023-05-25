#include "health_handler.h"

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>

#include "gtest/gtest.h"

TEST(HealthHandler, BasicSuccessHealthTest) {
  std::string req_data = "Sample data to be echoed";
  boost::beast::http::request<boost::beast::http::string_body> req{boost::beast::http::verb::get,  // GET
                                                                   "/health",                        // URI
                                                                   11};                            // HTTP 1.1
  req.body() = req_data;
  boost::beast::http::response<boost::beast::http::dynamic_body> res;
  HealthHandler handler;
  handler.HandleRequest(req, res);

  EXPECT_EQ(boost::beast::buffers_to_string(res.body().data()), "OK");
  EXPECT_EQ(res.version(), 11);
  EXPECT_EQ(res.result_int(), OK);
}