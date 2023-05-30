#ifndef STATIC_HANDLER_H
#define STATIC_HANDLER_H

#include <boost/beast/http.hpp>
#include <boost/filesystem.hpp>

#include "I_request_handler.h"

class StaticHandler : public I_RequestHandler {
 public:
  StaticHandler(std::string uri_path, const std::string& client_ip);
  StatusCode HandleRequest(const boost::beast::http::request<boost::beast::http::string_body> req,
                           boost::beast::http::response<boost::beast::http::dynamic_body>& res);
  StatusCode SetHeaders(const boost::beast::http::request<boost::beast::http::string_body> req,
                        boost::beast::http::response<boost::beast::http::dynamic_body>& res);

 private:
  boost::filesystem::path file_path_;
  const std::string handler_name = "StaticHandler";
};

#endif