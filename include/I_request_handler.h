#ifndef REQUEST_HANDLER_H
#define REQUEST_HANDLER_H

#include <boost/beast/http.hpp>
#include <cstdlib>
#include <iostream>
#include <string>


using namespace std;

enum StatusCode {
  OK = 200,
  BAD_REQUEST = 400,
  FORBIDDEN = 403,
  NOT_FOUND = 404,
  INTERNAL_SERVER_ERROR = 500
};

const unordered_map<StatusCode, pair<std::string, boost::beast::http::status>> status_code_map_ = {
    {OK, pair<std::string, boost::beast::http::status>("OK", boost::beast::http::status::ok)},
    {BAD_REQUEST, pair<std::string, boost::beast::http::status>("Bad Request", boost::beast::http::status::bad_request)},
    {FORBIDDEN, pair<std::string, boost::beast::http::status>("Forbidden", boost::beast::http::status::forbidden)},
    {NOT_FOUND, pair<std::string, boost::beast::http::status>("Not Found", boost::beast::http::status::not_found)},
    {INTERNAL_SERVER_ERROR, pair<std::string, boost::beast::http::status>("Internal Server Error", boost::beast::http::status::internal_server_error)}};
  

class I_RequestHandler {
 public:
  virtual StatusCode HandleRequest(const boost::beast::http::request<boost::beast::http::string_body> req, 
                             boost::beast::http::response<boost::beast::http::dynamic_body>& res) = 0;
};

#endif