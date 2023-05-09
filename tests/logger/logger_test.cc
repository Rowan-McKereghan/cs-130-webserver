#include "logger.h"

#include <gtest/gtest.h>

#include <boost/date_time/posix_time/posix_time_io.hpp>
#include <fstream>
#include <regex>
#include <sstream>
#include <string>

// Sets up logging identical to production
class TimestampedLoggerTest : public ::testing::Test {
 protected:
  static void SetUpTestCase() { InitLogging(); }

  static void TearDownTestCase() {}
};

auto fixed_log_file_path = "logs/server.log";

// Fixes log filepath for testing
class FixedPathLoggerTest : public ::testing::Test {
 protected:
  static void SetUpTestCase() { InitLogging(fixed_log_file_path); }

  static void TearDownTestCase() {}
};

// Verify single log to console
TEST_F(TimestampedLoggerTest, ConsoleOutput) {
  std::stringstream buffer;
  std::streambuf* oldClogStreamBuf = std::clog.rdbuf();
  std::clog.rdbuf(buffer.rdbuf());

  LOG(info) << "Test console output";

  std::clog.rdbuf(oldClogStreamBuf);
  std::string output = buffer.str();

  std::regex log_regex(
      R"(\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2}\.\d{6} \[.+\] <info> Test console output\n)");
  EXPECT_TRUE(std::regex_search(output, log_regex));
}

// Verify error log to console
TEST_F(TimestampedLoggerTest, ConsoleErrorOutput) {
  std::stringstream buffer;
  std::streambuf* oldClogStreamBuf = std::clog.rdbuf();
  std::clog.rdbuf(buffer.rdbuf());

  LOG(error) << "Test console output";

  std::clog.rdbuf(oldClogStreamBuf);
  std::string output = buffer.str();

  std::regex log_regex(
      R"(\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2}\.\d{6} \[.+\] <error> Test console output\n)");
  EXPECT_TRUE(std::regex_search(output, log_regex));
}

// Verify single log to file
TEST_F(FixedPathLoggerTest, FileOutput) {
  LOG(info) << "Test file output";

  std::ifstream log_file(fixed_log_file_path);
  std::string line;
  std::string last_line;

  while (std::getline(log_file, line)) {
    last_line = line;
  }

  std::regex log_regex(
      R"(\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2}\.\d{6} \[.+\] <info> Test file output)");
  EXPECT_TRUE(std::regex_search(last_line, log_regex));
}
