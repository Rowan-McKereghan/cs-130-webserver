#include "not_found_handler_factory.h"

#include "not_found_handler.h"

NotFoundHandler* NotFoundHandlerFactory::CreateHandler(std::string& file_path) {
  return new NotFoundHandler();
}