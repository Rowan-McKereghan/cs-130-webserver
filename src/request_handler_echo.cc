#include "request_handler_echo.h"
void RequestHandlerEcho::HandleRequest(const Request &req, Response &res) {
  // Set the response status code
  res.status_code = 200;
  // Set the response content type
  HTTPHeader content_type_header;
  content_type_header.name = "Content-Type";
  content_type_header.value = "text/plain";
  res.headers.push_back(content_type_header);
  // Set the response data to the original request string
  res.data = req.raw_request;
  // Set the Content-Length header based on the response data size
  // TODO: Verify implementation
  HTTPHeader content_length_header;
  content_length_header.name = "Content-Length";
  content_length_header.value = std::to_string(res.data.size());
  res.headers.push_back(content_length_header);
}