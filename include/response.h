#ifndef RESPONSE_H
#define RESPONSE_H

#include <boost/asio.hpp>
#include <boost/filesystem.hpp>
#include <string>
#include <unordered_map>
#include <vector>

#include "HTTPHeader.h"

enum StatusCode {
  OK = 200,
  BAD_REQUEST = 400,
  FORBIDDEN = 403,
  NOT_FOUND = 404,
  INTERNAL_SERVER_ERROR = 500
};

class Response {
 public:
  const std::string CRLF = "\r\n";
  Response(boost::asio::ip::tcp::socket* socket);

  void set_status_code(StatusCode code) { status_code_ = code; }
  void set_body(const std::string& data) { data_ = data; }
  void set_error_response(StatusCode code);
  void add_header(const std::string& name, const std::string& value) {
    headers_.emplace_back(HTTPHeader{name, value});
  }
  StatusCode get_status_code() { return status_code_; }
  std::vector<HTTPHeader> get_headers() { return headers_; }
  std::string get_data() { return data_; }
  bool get_wrote_http_response() { return wrote_http_response_; }
  void WriteHTTPResponse();
  std::string GenerateHTTPResponse();
  void WriteToSocket(boost::asio::const_buffer buffer);

 private:
  StatusCode status_code_;
  std::vector<HTTPHeader> headers_;
  std::string data_;
  boost::asio::ip::tcp::socket* socket_;
  bool wrote_http_response_;
  static const std::unordered_map<StatusCode, std::string> status_code_map_;

  std::string GenerateStatusLine();
  std::string GenerateDateHeader();
  void HandleWrite(const boost::system::error_code& error);
};

#endif
