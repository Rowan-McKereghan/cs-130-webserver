#ifndef STATIC_HANDLER_H
#define STATIC_HANDLER_H

#include <boost/beast/http.hpp>
#include <boost/filesystem.hpp>

#include "I_request_handler.h"

class StaticHandler : public I_RequestHandler {
 public:
  StaticHandler(std::string file_path);
  StatusCode HandleRequest(boost::beast::http::request<boost::beast::http::string_body> req, 
                     boost::beast::http::response<boost::beast::http::dynamic_body>& res);
  StatusCode SetHeaders(boost::beast::http::request<boost::beast::http::string_body> req, 
                 boost::beast::http::response<boost::beast::http::dynamic_body>& res);

 private:
  boost::filesystem::path file_path_;
};

#endif