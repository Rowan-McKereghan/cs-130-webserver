#ifndef STATIC_HANDLER_H
#define STATIC_HANDLER_H

#include <boost/filesystem.hpp>

#include "I_request_handler.h"

class StaticHandler : public I_RequestHandler {
 public:
  StaticHandler(std::string file_path);
  void HandleRequest(const Request &req, Response &res);
  int SetHeaders(const Request &req, Response &res);

 private:
  boost::filesystem::path file_path_;
};

#endif