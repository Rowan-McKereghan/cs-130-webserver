#include "health_handler.h"

#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <string>

#include "logger.h"

HealthHandler::HealthHandler(const std::string& client_ip) {
	this->client_ip = client_ip;
}

StatusCode HealthHandler::HandleRequest(const boost::beast::http::request<boost::beast::http::string_body> req,
                                        boost::beast::http::response<boost::beast::http::dynamic_body>& res) {
  res.result(boost::beast::http::status::ok);
  res.version(req.version());
  res.set(boost::beast::http::field::server, "webserver");
  boost::beast::ostream(res.body()) << "OK";
  res.set(boost::beast::http::field::content_type, "text/plain");
  LOG(info) << log_magic << "Response code: " << boost::beast::http::status::ok << " Request path: " << req.target() << " Request IP: " << client_ip << " Handler Name: " << handler_name;
  res.prepare_payload();
  return OK;
}