#include "response.h"

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/filesystem.hpp>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "logger.h"

const std::unordered_map<StatusCode, std::string> Response::status_code_map_ = {
    {OK, "OK"},
    {BAD_REQUEST, "Bad Request"},
    {NOT_FOUND, "Not Found"},
    {INTERNAL_SERVER_ERROR, "Internal Server Error"}};

void Response::write_http_response() {
  if (!has_written_http_response_) {
    std::string response = generate_http_response();
    write_to_socket(
        boost::asio::const_buffer(response.data(), response.size()));
    has_written_http_response_ = true;
  } else {
    LOG(info) << "Attempted to write a 2nd HTTP Response. This is normal for "
                 "requests for static files";
  }
}

std::string Response::generate_http_response() {
  std::ostringstream res;

  res << generate_status_line();
  res << generate_date_header();
  res << "Server: webserver\r\n";

  // Set Headers
  std::ostringstream headers_stream;
  for (const auto& header : headers_) {
    headers_stream << header.name << ": " << header.value << CRLF;
  }

  res << headers_stream.str();
  res << CRLF;

  res << data_;  // Set body

  return res.str();
}

std::string Response::generate_status_line() {
  std::ostringstream status_stream;

  status_stream << "HTTP/1.1 " << std::to_string(status_code_) << " ";

  auto it = status_code_map_.find(status_code_);
  if (it != status_code_map_.end()) {
    status_stream << it->second;
  } else {
    status_stream << "Unknown Status Code";
    LOG(warning) << "Unkonwn";
  }

  status_stream << CRLF;
  return status_stream.str();
}

std::string Response::generate_date_header() {
  time_t t;
  struct tm* myTime;
  char date[100];
  t = time(NULL);
  myTime = gmtime(&t);
  strftime(date, sizeof(date), "Date: %a, %d %b %G %T GMT", myTime);

  return std::string(date) + CRLF;
}

void Response::write_to_socket(boost::asio::const_buffer buffer) {
  boost::asio::async_write(*socket_, buffer,
                           boost::bind(&Response::handle_write, this,
                                       boost::asio::placeholders::error));
}

void Response::handle_write(const boost::system::error_code& error) {
  if (error != boost::system::errc::success) {
    log_error(error, "An error occurred in handle_write");
  }
}