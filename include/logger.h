#ifndef LOGGER_H
#define LOGGER_H

#include <boost/log/trivial.hpp>

void init_logging();
void init_logging(const std::string& log_file_path);

#define LOG(severity) BOOST_LOG_TRIVIAL(severity)

#endif