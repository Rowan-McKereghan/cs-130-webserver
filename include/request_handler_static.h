#ifndef REQUEST_HANDLER_STATIC_H
#define REQUEST_HANDLER_ECHO_H

#include <boost/filesystem.hpp>

#include "request_handler.h"

class RequestHandlerStatic : public I_RequestHandler {
 public:
  RequestHandlerStatic(boost::filesystem::path root);
  void HandleRequest(const Request &req, Response &res);
  int HandleRequestHelper(const Request &req, Response &res);

 private:
  boost::filesystem::path file_path_;
};

#endif