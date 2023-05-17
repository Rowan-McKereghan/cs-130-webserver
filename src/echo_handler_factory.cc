#include "echo_handler_factory.h"

#include "echo_handler.h"

EchoHandler* EchoHandlerFactory::CreateHandler(std::string& file_path) { return new EchoHandler(); }