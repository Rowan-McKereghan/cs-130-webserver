#include "sleep_handler_factory.h"

#include "sleep_handler.h"

SleepHandler* SleepHandlerFactory::CreateHandler(std::string& file_path) { return new SleepHandler(); }