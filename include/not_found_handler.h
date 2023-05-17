#ifndef NOT_FOUND_HANDLER_H
#define NOT_FOUND_HANDLER_H

#include <boost/beast/http.hpp>

#include "I_request_handler.h"

class NotFoundHandler : public I_RequestHandler {
 public:
  // No constructor needed since it is config invariant
  StatusCode HandleRequest(
      const boost::beast::http::request<boost::beast::http::string_body> req,
      boost::beast::http::response<boost::beast::http::dynamic_body>& res)
      override;
};

#endif