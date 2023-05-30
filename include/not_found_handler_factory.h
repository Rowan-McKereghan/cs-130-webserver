#ifndef NOT_FOUND_HANDLER_FACTORY_H
#define NOT_FOUND_HANDLER_FACTORY_H
#include <string>

#include "I_request_handler_factory.h"
#include "config_parser.h"
#include "not_found_handler.h"

class NotFoundHandlerFactory : public I_RequestHandlerFactory {
 public:
  NotFoundHandler* CreateHandler(std::string& uri_path, const std::string& client_ip);
};
#endif