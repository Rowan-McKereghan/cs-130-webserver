cmake_minimum_required(VERSION 3.10.0)
project(ctrl-c-ctrl-v)
# Make sure developers do not run cmake in the main project directory, to keep
# build artifacts from becoming clutter
if (${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
    message(FATAL_ERROR "In-source builds not allowed.
    Please make a new directory (called a build directory) and run CMake from there.
    You may need to remove CMakeCache.txt." )
endif()
# Turn on debug builds if we are building in a devel shell
if (CMAKE_BUILD_TYPE STREQUAL "" AND "$ENV{DEVEL_SHELL}" STREQUAL "1")
    message(STATUS "Setting debug build type by default in devel shell")
    set(CMAKE_BUILD_TYPE Debug)
endif()
# Output binaries to a sub directory "bin"
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
# Enable cmake testing
include(CTest)
enable_testing()
# Enable GoogleTest
include(GoogleTest)
add_subdirectory(/usr/src/googletest googletest)

# Enable Boost
# Use static libraries so binaries can be deployed without a full boost install
set(Boost_USE_STATIC_LIBS ON)
find_package(Boost 1.50 REQUIRED COMPONENTS system log log_setup)
message(STATUS "Boost version: ${Boost_VERSION}")

include_directories(include)

add_library(server_lib src/server.cc)
add_library(session_lib src/session.cc)
add_library(request_processor_lib src/request_processor.cc)
add_library(request_handler_echo_lib src/request_handler_echo.cc)
add_library(request_lib src/request.cc)
add_library(request_handler_static_lib src/request_handler_static.cc)
add_library(response_lib src/response.cc)
add_library(config_parser_lib src/config_parser.cc)
add_library(logger_lib src/logger.cc)
add_library(serving_config_lib src/serving_config.cc)

add_executable(webserver src/server_main.cc)
target_link_libraries(webserver server_lib session_lib config_parser_lib logger_lib serving_config_lib ${Boost_LIBRARIES})
target_link_libraries(server_lib logger_lib ${Boost_LIBRARIES})
target_link_libraries(session_lib logger_lib request_processor_lib response_lib ${Boost_LIBRARIES})
target_link_libraries(request_processor_lib request_handler_echo_lib request_handler_static_lib request_lib ${Boost_LIBRARIES})
target_link_libraries(request_handler_static_lib logger_lib response_lib ${Boost_LIBRARIES})
target_link_libraries(request_lib logger_lib ${Boost_LIBRARIES})
target_link_libraries(response_lib logger_lib request_handler_static_lib ${Boost_LIBRARIES})
target_link_libraries(serving_config_lib logger_lib config_parser_lib ${Boost_LIBRARIES})

add_executable(config_parser_test tests/config_parser/config_parser_test.cc)
add_executable(webserver_test tests/server/server_test.cc)
add_executable(session_test tests/session/session_test.cc)
add_executable(logger_test tests/logger/logger_test.cc)
add_executable(request_test tests/request/request_test.cc)
add_executable(request_processor_test tests/request/request_processor_test.cc)
add_executable(request_handler_echo_test tests/request/request_handler_echo_test.cc)
add_executable(request_handler_static_test tests/request/request_handler_static_test.cc)
add_executable(serving_config_test tests/serving_config/serving_config_test.cc)

target_link_libraries(webserver_test server_lib logger_lib gtest_main gmock_main ${Boost_LIBRARIES})
target_link_libraries(session_test session_lib logger_lib gtest_main gmock_main ${Boost_LIBRARIES})
target_link_libraries(request_test logger_lib request_processor_lib request_lib gtest_main ${Boost_LIBRARIES})
target_link_libraries(request_processor_test request_processor_lib logger_lib gtest_main gmock_main ${Boost_LIBRARIES})
target_link_libraries(request_handler_static_test request_handler_static_lib logger_lib gtest_main ${Boost_LIBRARIES})
target_link_libraries(config_parser_test config_parser_lib gtest_main ${Boost_LIBRARIES})
target_link_libraries(logger_test logger_lib gtest_main gmock_main ${Boost_LIBRARIES})
target_link_libraries(request_handler_echo_test request_handler_echo_lib gtest_main ${Boost_LIBRARIES})
target_link_libraries(serving_config_test serving_config_lib gtest_main ${Boost_LIBRARIES})

add_test(NAME integration_test
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/tests/integration_test.sh
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)

gtest_discover_tests(config_parser_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)
gtest_discover_tests(webserver_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)
gtest_discover_tests(session_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)
gtest_discover_tests(logger_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)
gtest_discover_tests(request_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)
gtest_discover_tests(request_processor_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)
gtest_discover_tests(request_handler_echo_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)
gtest_discover_tests(request_handler_static_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)
gtest_discover_tests(serving_config_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)

include(cmake/CodeCoverageReportConfig.cmake)
generate_coverage_report(TARGETS webserver server_lib session_lib config_parser_lib logger_lib config_parser_test webserver_test session_test TESTS config_parser_test webserver_test session_test logger_test request_test request_processor_test request_handler_echo_test request_handler_static_test serving_config_test)