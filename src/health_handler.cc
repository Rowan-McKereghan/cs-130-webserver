#include "health_handler.h"

#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <string>

#include "logger.h"

StatusCode HealthHandler::HandleRequest(const boost::beast::http::request<boost::beast::http::string_body> req,
                                      boost::beast::http::response<boost::beast::http::dynamic_body>& res) {
    
    res.result(boost::beast::http::status::ok);
    res.version(req.version());
    res.set(boost::beast::http::field::server, "webserver");
    boost::beast::ostream(res.body()) << "OK";
    res.set(boost::beast::http::field::content_type, "text/plain");
    res.prepare_payload();
    return OK;
  
}