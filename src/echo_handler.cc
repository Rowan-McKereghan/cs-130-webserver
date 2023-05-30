#include "echo_handler.h"

#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/lexical_cast.hpp>
#include <string>

#include "logger.h"

EchoHandler::EchoHandler(const std::string& client_ip) {
	this->client_ip = client_ip;
}

StatusCode EchoHandler::HandleRequest(const boost::beast::http::request<boost::beast::http::string_body> req,
                                      boost::beast::http::response<boost::beast::http::dynamic_body>& res) {
  res.result(OK);
  res.version(req.version());
  res.set(boost::beast::http::field::server, "webserver");
  boost::beast::ostream(res.body()) << req;
  LOG(info) << log_magic << "Response code: " << OK << " Request path: " << req.target() << " Request IP: " << client_ip << " Handler Name: " << handler_name;
  res.prepare_payload();
  res.set(boost::beast::http::field::content_type, "text/plain");
  return OK;
}