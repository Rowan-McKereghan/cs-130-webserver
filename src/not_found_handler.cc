#include "not_found_handler.h"

#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/lexical_cast.hpp>
#include <string>

#include "logger.h"

StatusCode NotFoundHandler::HandleRequest(
    const boost::beast::http::request<boost::beast::http::string_body> req,
    boost::beast::http::response<boost::beast::http::dynamic_body>& res) {
  res.result(NOT_FOUND);
  res.version(req.version());
  res.set(boost::beast::http::field::server, "webserver");
  boost::beast::ostream(res.body()) << "404 Not Found";
  res.prepare_payload();
  res.set(boost::beast::http::field::content_type, "text/plain");
  return NOT_FOUND;
}