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
find_package(Boost 1.50 REQUIRED COMPONENTS system)
message(STATUS "Boost version: ${Boost_VERSION}")

include_directories(include)

# Update name and srcs
add_library(server_lib src/server.cc)
add_library(session_lib src/session.cc)
add_library(config_parser_lib src/config_parser.cc)

# Update executable name, srcs, and deps
add_executable(webserver src/server_main.cc)
target_link_libraries(webserver Boost::system)
target_link_libraries(webserver server_lib)
target_link_libraries(webserver session_lib)
target_link_libraries(webserver config_parser_lib)

# Update test executable name, srcs, and deps
add_executable(config_parser_test tests/config_parser/config_parser_test.cc)
add_executable(webserver_test tests/server/server_test.cc)
add_executable(session_test tests/session/session_test.cc)
target_link_libraries(config_parser_test config_parser_lib gtest_main)
target_link_libraries(webserver_test server_lib gtest_main gmock_main)
target_link_libraries(session_test session_lib gtest_main gmock_main)

# Add integration test script

add_test(NAME integration_test
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/tests/integration_test.sh
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)


# Update with test binary
gtest_discover_tests(config_parser_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)
gtest_discover_tests(webserver_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)
gtest_discover_tests(session_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)

# Update with target/test targets
include(cmake/CodeCoverageReportConfig.cmake)
generate_coverage_report(TARGETS webserver server_lib session_lib config_parser_lib config_parser_test webserver_test session_test TESTS config_parser_test webserver_test session_test)