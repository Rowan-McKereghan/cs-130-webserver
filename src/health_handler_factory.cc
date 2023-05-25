#include "health_handler_factory.h"

#include "health_handler.h"

HealthHandler* HealthHandlerFactory::CreateHandler(std::string& file_path) { return new HealthHandler(); }