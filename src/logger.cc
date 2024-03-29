#include "logger.h"

#include <boost/log/expressions.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/file.hpp>

namespace logging = boost::log;
namespace expr = logging::expressions;
namespace keywords = logging::keywords;

// Common log format for both console and file sinks
auto log_format =
    (expr::stream << expr::format_date_time<boost::posix_time::ptime>("TimeStamp", "%Y-%m-%d %H:%M:%S.%f") << " ["
                  << expr::attr<logging::attributes::current_thread_id::value_type>("ThreadID") << "]"
                  << " <" << logging::trivial::severity << "> " << expr::smessage);

const auto kMaxLogFileSizeBytes = 10 * 1024 * 1024;

void InitLogging(const std::string& log_file_path) {
  logging::add_common_attributes();

  // Setup console sink
  auto console_sink = logging::add_console_log(std::clog, keywords::format = log_format);

  // Setup file sink with rotation
  auto file_sink = logging::add_file_log(
      keywords::file_name = log_file_path,
      keywords::rotation_size = kMaxLogFileSizeBytes,  // Rotate if the log reaches 10MB
      keywords::time_based_rotation = logging::sinks::file::rotation_at_time_point(0, 0, 0),  // Rotate at midnight
      keywords::auto_flush = true, keywords::format = log_format);

  // Set minimum severity level to info (e.g., emit logs at info level or above)
  logging::core::get()->set_filter(logging::trivial::severity >= logging::trivial::info);
}

std::string FormatError(const boost::system::error_code& error) {
  std::ostringstream oss;
  oss << error.category().name() << "::" << error.message() << " (" << error.value() << ")";
  return oss.str();
}

void LogError(const boost::system::error_code& ec, const std::string& message) {
  BOOST_LOG_TRIVIAL(error) << message << FormatError(ec);
}