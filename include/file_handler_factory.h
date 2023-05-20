#ifndef FILE_HANDLER_FACTORY_H
#define FILE_HANDLER_FACTORY_H

#include <boost/beast/http.hpp>
#include <boost/filesystem.hpp>

#include "I_request_handler_factory.h"

class FileHandlerFactory : public I_RequestHandlerFactory {
 public:
  virtual I_RequestHandler* CreateHandler(std::string& uri_path) = 0;
  std::string GetCompleteFilePath(NginxConfig* config, std::string& file_path);
  std::string GetRoot(NginxConfig* config);
};

#endif