#include "not_found_handler_factory.h"

#include "not_found_handler.h"

NotFoundHandler* NotFoundHandlerFactory::CreateHandler(std::string& file_path, const std::string& client_ip) { return new NotFoundHandler(client_ip); }