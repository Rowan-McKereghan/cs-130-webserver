#ifndef HEALTH_HANDLER_H
#define HEALTH_HANDLER_H

#include <boost/beast/http.hpp>

#include "I_request_handler.h"

// HealthHandler is meant to return OK at all times -- if it doesn't, our server is not healthy.
class HealthHandler : public I_RequestHandler {
 public:
  HealthHandler(const std::string& client_ip);
  StatusCode HandleRequest(const boost::beast::http::request<boost::beast::http::string_body> req,
                           boost::beast::http::response<boost::beast::http::dynamic_body>& res) override;
 private:
  const std::string handler_name = "HealthHandler";
};

#endif