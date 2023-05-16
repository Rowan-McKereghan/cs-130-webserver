#ifndef ECHO_HANDLER_H
#define ECHO_HANDLER_H

#include "I_request_handler.h"

#include <boost/beast/http.hpp>

class EchoHandler : public I_RequestHandler {
 public:
  // No constructor needed since it is config invariant
  StatusCode HandleRequest(boost::beast::http::request<boost::beast::http::string_body> req, 
                     boost::beast::http::response<boost::beast::http::dynamic_body>& res) override;
};

#endif