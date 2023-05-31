#ifndef LOGGER_H
#define LOGGER_H

#include <boost/log/trivial.hpp>
#include <boost/system/error_code.hpp>

// This file defines common logging functions including disk and console logging

const std::string response_log_magic = "[ResponseMetrics]";

// Sets up logging to console and file
void InitLogging(const std::string& log_file_path = "logs/server_%Y_%m_%d_%H%M%S_%5N.log");

std::string FormatError(const boost::system::error_code& error);

// Convinience function for boost error codes
void LogError(const boost::system::error_code& ec, const std::string& message);

// Current valid values (from boost::log::trivial::severity_level) are trace,
// debug, info, warning, error, fatal
#define LOG(severity) BOOST_LOG_TRIVIAL(severity)

#endif