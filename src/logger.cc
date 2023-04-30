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
    (expr::stream
     << expr::format_date_time<boost::posix_time::ptime>("TimeStamp",
                                                         "%Y-%m-%d %H:%M:%S.%f")
     << " ["
     << expr::attr<logging::attributes::current_thread_id::value_type>(
            "ThreadID")
     << "]"
     << " <" << logging::trivial::severity << "> " << expr::smessage);

auto log_file_name_format = "logs/server_%Y_%m_%d_%H%M%S_%5N.log";

void init_logging() { init_logging(log_file_name_format); }

void init_logging(const std::string& log_file_path) {
  logging::add_common_attributes();

  // Setup console sink
  auto console_sink =
      logging::add_console_log(std::clog, keywords::format = log_format);

  // Setup file sink with rotation
  auto file_sink = logging::add_file_log(
      keywords::file_name = log_file_path,
      keywords::rotation_size =
          10 * 1024 * 1024,  // Rotate if the log reaches 10MB
      keywords::time_based_rotation =
          logging::sinks::file::rotation_at_time_point(
              0, 0, 0),  // Rotate at midnight
      keywords::auto_flush = true, keywords::format = log_format);

  // Set minimum severity level to trace (e.g., emit all logs)
  logging::core::get()->set_filter(logging::trivial::severity >=
                                   logging::trivial::trace);
}
