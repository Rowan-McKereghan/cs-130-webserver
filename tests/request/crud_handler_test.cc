#include "crud_handler.h"

#include <boost/beast/core.hpp>

#include "gtest/gtest.h"

// tests HandleRequest with an unsupported HTTP method
TEST(CrudHandler, CrudTestUnsuportedMethod) {
  string req_data = "Irrelevant stuff in request body";
  boost::beast::http::request<boost::beast::http::string_body> req{
      boost::beast::http::verb::connect,  // Unsupported method
      "/api/entity/1",                    // URI
      11};                                // HTTP 1.1
  req.body() = req_data;
  boost::beast::http::response<boost::beast::http::dynamic_body> res;
  std::string root = "/crud";
  unordered_map<std::string, std::list<int>> file_to_id;
  CrudHandler handler(root, file_to_id);
  handler.HandleRequest(req, res);
  EXPECT_EQ(res.version(), 11);                                                      // version
  EXPECT_EQ(res.result_int(), BAD_REQUEST);                                          // status
  EXPECT_EQ(boost::beast::buffers_to_string(res.body().data()), "400 Bad Request");  // body
}