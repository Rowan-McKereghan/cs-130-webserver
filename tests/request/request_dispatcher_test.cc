#include "request_dispatcher.h"

#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/filesystem.hpp>

#include "echo_handler.h"
#include "echo_handler_factory.h"
#include "gtest/gtest.h"
#include "not_found_handler.h"
#include "not_found_handler_factory.h"
#include "static_handler.h"
#include "static_handler_factory.h"

TEST(RequestDispatcherTest, RouteRequest) {
  boost::beast::http::request<boost::beast::http::string_body> req{boost::beast::http::verb::get,  // GET
                                                                   "/echo2?ignore=param",          // URI
                                                                   11};                            // HTTP 1.1
  req.set(boost::beast::http::field::host, "localhost");
  req.set(boost::beast::http::field::user_agent,
          "Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:108.0) Gecko/20100101 Firefox/108.0");
  req.set(boost::beast::http::field::accept,
          "text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8");
  req.set(boost::beast::http::field::accept_language, "en-US,en;q=0.5");
  req.set(boost::beast::http::field::accept_encoding, "gzip, deflate");
  req.set(boost::beast::http::field::connection, "keep-alive");

  boost::beast::http::response<boost::beast::http::dynamic_body> res;
  RequestDispatcher req_dispatcher;
  EchoHandlerFactory* echo_handler_factory = new EchoHandlerFactory();
  ServingConfig serving_config{                                      // echo_paths
                               {{"/echo2", echo_handler_factory}}};  // handler_factories
  req_dispatcher.RouteRequest(req, res, serving_config, "127.0.0.1");
  boost::beast::http::dynamic_body::value_type req_body;
  boost::beast::ostream(req_body) << req;
  EXPECT_EQ(res.result_int(), OK);
  EXPECT_EQ(res.version(), 11);
  EXPECT_EQ(boost::beast::buffers_to_string(res.body().data()), boost::beast::buffers_to_string(req_body.data()));
}

TEST(RequestParserTest, InvalidRequestType) {
  boost::beast::http::request<boost::beast::http::string_body> req{boost::beast::http::verb::get,  // GET
                                                                   "/invalid_uri",                 // URI
                                                                   11};                            // HTTP 1.1
  req.set(boost::beast::http::field::host, "localhost");
  req.set(boost::beast::http::field::user_agent,
          "Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:108.0) Gecko/20100101 Firefox/108.0");
  req.set(boost::beast::http::field::accept,
          "text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8");
  req.set(boost::beast::http::field::accept_language, "en-US,en;q=0.5");
  req.set(boost::beast::http::field::accept_encoding, "gzip, deflate");
  req.set(boost::beast::http::field::connection, "keep-alive");
  boost::beast::http::response<boost::beast::http::dynamic_body> res;
  RequestDispatcher req_dispatcher;
  EchoHandlerFactory* echo_handler_factory = new EchoHandlerFactory();
  ServingConfig serving_config{{{"/echo2", echo_handler_factory}}};  // handler_factories
  std::string client_ip = "127.0.0.1";
  req_dispatcher.RouteRequest(req, res, serving_config, client_ip);
  EXPECT_EQ(res.result_int(), BAD_REQUEST);
  EXPECT_EQ(res.version(), 11);
  delete echo_handler_factory;
}

// Test URI without a leading slash
TEST(RequestDispatcherTest, BaseURIRequestTest) {
  boost::beast::http::request<boost::beast::http::string_body> req{boost::beast::http::verb::get,  // GET
                                                                   "",                             // URI
                                                                   11};                            // HTTP 1.1

  boost::beast::http::response<boost::beast::http::dynamic_body> res;
  RequestDispatcher req_dispatcher;
  NotFoundHandlerFactory* not_found_handler_factory = new NotFoundHandlerFactory();
  ServingConfig serving_config{                                      // echo_paths
                               {{"/", not_found_handler_factory}}};  // handler_factories
  req_dispatcher.RouteRequest(req, res, serving_config, "127.0.0.1");
  EXPECT_EQ(res.result_int(), NOT_FOUND);
  EXPECT_EQ(res.version(), 11);
}

// Test URI without a leading slash
TEST(RequestDispatcherTest, WrongHTTPVersionRequestTest) {
  boost::beast::http::request<boost::beast::http::string_body> req{boost::beast::http::verb::get,  // GET
                                                                   "",                             // URI
                                                                   20};                            // HTTP 2.0

  boost::beast::http::response<boost::beast::http::dynamic_body> res;
  RequestDispatcher req_dispatcher;
  ServingConfig serving_config{{}};  // handler_factories
  req_dispatcher.RouteRequest(req, res, serving_config, "127.0.0.1");
  EXPECT_EQ(res.result_int(), BAD_REQUEST);
  EXPECT_EQ(res.version(), 11);
}

// Test Longest Prefix matching (e.g., for static files)
TEST(RequestDispatcherTest, LongPathMatchingRequestTest) {
  boost::beast::http::request<boost::beast::http::string_body> req{boost::beast::http::verb::get,  // GET
                                                                   "/static//index.html",          // URI
                                                                   11};                            // HTTP 1.1

  boost::beast::http::response<boost::beast::http::dynamic_body> res;
  RequestDispatcher req_dispatcher;
  NginxConfig config;
  StaticHandlerFactory* static_handler_factory = new StaticHandlerFactory(&config);
  ServingConfig serving_config{{{"/static", static_handler_factory}}};  // handler_factories
  req_dispatcher.RouteRequest(req, res, serving_config, "127.0.0.1");
  EXPECT_EQ(res.result_int(), NOT_FOUND);
  EXPECT_EQ(res.version(), 11);
}