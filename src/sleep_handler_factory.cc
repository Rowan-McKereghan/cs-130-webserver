#include "sleep_handler_factory.h"

#include "sleep_handler.h"

SleepHandler* SleepHandlerFactory::CreateHandler(std::string& file_path, const std::string& client_ip) { return new SleepHandler(client_ip); }