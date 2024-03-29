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
add_library(request_dispatcher_lib src/request_dispatcher.cc)
add_library(echo_handler_lib src/echo_handler.cc)
add_library(static_handler_lib src/static_handler.cc)
add_library(config_parser_lib src/config_parser.cc)
add_library(logger_lib src/logger.cc)
add_library(serving_config_lib src/serving_config.cc)
add_library(static_handler_factory_lib src/static_handler_factory.cc)
add_library(websocket_info_handler_lib src/websocket_info_handler.cc)
add_library(websocket_info_handler_factory_lib src/websocket_info_handler_factory.cc)
add_library(echo_handler_factory_lib src/echo_handler_factory.cc)
add_library(not_found_handler_lib src/not_found_handler.cc)
add_library(not_found_handler_factory_lib src/not_found_handler_factory.cc)
add_library(sleep_handler_lib src/sleep_handler.cc)
add_library(sleep_handler_factory_lib src/sleep_handler_factory.cc)
add_library(file_handler_factory_lib src/I_file_handler_factory.cc)
add_library(crud_handler_lib src/crud_handler.cc)
add_library(crud_handler_factory_lib src/crud_handler_factory.cc)
add_library(crud_filesystem_manager_lib src/crud_filesystem_manager.cc)
add_library(health_handler_lib src/health_handler.cc)
add_library(health_handler_factory_lib src/health_handler_factory.cc)
add_library(websocket_handler_lib src/websocket_handler.cc)
add_library(global_websocket_state_lib src/global_websocket_state.cc)


add_executable(webserver src/server_main.cc)
target_link_libraries(webserver server_lib session_lib config_parser_lib logger_lib serving_config_lib global_websocket_state_lib ${Boost_LIBRARIES})
target_link_libraries(server_lib logger_lib global_websocket_state_lib ${Boost_LIBRARIES})
target_link_libraries(session_lib logger_lib request_dispatcher_lib global_websocket_state_lib websocket_handler_lib ${Boost_LIBRARIES})
target_link_libraries(request_dispatcher_lib echo_handler_lib static_handler_lib echo_handler_factory_lib static_handler_factory_lib not_found_handler_factory_lib ${Boost_LIBRARIES})
target_link_libraries(static_handler_lib logger_lib ${Boost_LIBRARIES})
target_link_libraries(global_websocket_state_lib logger_lib ${Boost_LIBRARIES})
target_link_libraries(serving_config_lib logger_lib config_parser_lib echo_handler_factory_lib websocket_info_handler_factory_lib static_handler_factory_lib not_found_handler_factory_lib crud_handler_factory_lib health_handler_factory_lib sleep_handler_factory_lib ${Boost_LIBRARIES})
target_link_libraries(static_handler_factory_lib logger_lib config_parser_lib static_handler_lib file_handler_factory_lib ${Boost_LIBRARIES})
target_link_libraries(echo_handler_factory_lib logger_lib config_parser_lib echo_handler_lib ${Boost_LIBRARIES})
target_link_libraries(websocket_info_handler_factory_lib logger_lib websocket_info_handler_lib global_websocket_state_lib ${Boost_LIBRARIES})
target_link_libraries(not_found_handler_lib logger_lib ${Boost_LIBRARIES})
target_link_libraries(not_found_handler_factory_lib logger_lib not_found_handler_lib ${Boost_LIBRARIES})
target_link_libraries(sleep_handler_lib logger_lib ${Boost_LIBRARIES})
target_link_libraries(sleep_handler_factory_lib sleep_handler_lib logger_lib ${Boost_LIBRARIES})
target_link_libraries(crud_handler_lib logger_lib crud_filesystem_manager_lib ${Boost_LIBRARIES})
target_link_libraries(crud_handler_factory_lib logger_lib crud_handler_lib file_handler_factory_lib crud_filesystem_manager_lib ${Boost_LIBRARIES})
target_link_libraries(crud_filesystem_manager_lib ${Boost_LIBRARIES})
target_link_libraries(health_handler_factory_lib logger_lib config_parser_lib health_handler_lib ${Boost_LIBRARIES})
target_link_libraries(websocket_handler_lib logger_lib ${Boost_LIBRARIES})
target_link_libraries(global_websocket_state_lib logger_lib ${Boost_LIBRARIES})


add_executable(config_parser_test tests/config_parser/config_parser_test.cc)
add_executable(webserver_test tests/server/server_test.cc)
add_executable(session_test tests/session/session_test.cc)
add_executable(logger_test tests/logger/logger_test.cc)
add_executable(request_dispatcher_test tests/request/request_dispatcher_test.cc)
add_executable(echo_handler_test tests/request/echo_handler_test.cc)
add_executable(static_handler_test tests/request/static_handler_test.cc)
add_executable(serving_config_test tests/serving_config/serving_config_test.cc)
add_executable(not_found_handler_test tests/request/not_found_handler_test.cc)
add_executable(not_found_handler_factory_test tests/factories/not_found_handler_factory_test.cc)
add_executable(crud_handler_test tests/request/crud_handler_test.cc)
add_executable(file_handler_factory_test tests/factories/I_file_handler_factory_test.cc)
add_executable(crud_handler_factory_test tests/factories/crud_handler_factory_test.cc)
add_executable(crud_filesystem_manager_test tests/filesystem_managers/crud_filesystem_manager_test.cc)
add_executable(health_handler_test tests/request/health_handler_test.cc)
add_executable(websocket_handler_test tests/websocket/websocket_handler_test.cc)
add_executable(websocket_info_handler_test tests/request/websocket_info_handler_test.cc)
add_executable(websocket_info_handler_factory_test tests/factories/websocket_info_handler_factory_test.cc)
add_executable(global_websocket_state_test tests/websocket/global_websocket_state_test.cc)

target_link_libraries(webserver_test server_lib session_lib logger_lib gtest_main gmock_main ${Boost_LIBRARIES})
target_link_libraries(session_test session_lib logger_lib gtest_main gmock_main ${Boost_LIBRARIES})
target_link_libraries(request_dispatcher_test request_dispatcher_lib logger_lib gtest_main gmock_main ${Boost_LIBRARIES})
target_link_libraries(static_handler_test static_handler_lib logger_lib gtest_main ${Boost_LIBRARIES})
target_link_libraries(config_parser_test config_parser_lib gtest_main ${Boost_LIBRARIES})
target_link_libraries(logger_test logger_lib gtest_main gmock_main ${Boost_LIBRARIES})
target_link_libraries(echo_handler_test echo_handler_lib logger_lib gtest_main ${Boost_LIBRARIES})
target_link_libraries(websocket_info_handler_factory_test websocket_info_handler_factory_lib gtest_main ${Boost_LIBRARIES})
target_link_libraries(serving_config_test serving_config_lib global_websocket_state_lib websocket_handler_lib logger_lib gtest_main ${Boost_LIBRARIES})
target_link_libraries(not_found_handler_test not_found_handler_lib logger_lib gtest_main ${Boost_LIBRARIES})
target_link_libraries(not_found_handler_factory_test not_found_handler_factory_lib logger_lib gtest_main ${Boost_LIBRARIES})
target_link_libraries(crud_handler_test crud_handler_lib logger_lib gtest_main gmock_main ${Boost_LIBRARIES})
target_link_libraries(file_handler_factory_test file_handler_factory_lib logger_lib gtest_main ${Boost_LIBRARIES})
target_link_libraries(crud_handler_factory_test crud_handler_factory_lib static_handler_factory_lib gtest_main gmock_main ${Boost_LIBRARIES})
target_link_libraries(crud_filesystem_manager_test crud_filesystem_manager_lib logger_lib gtest_main ${Boost_LIBRARIES})
target_link_libraries(health_handler_test health_handler_lib logger_lib gtest_main ${Boost_LIBRARIES})
target_link_libraries(websocket_handler_test websocket_handler_lib global_websocket_state_lib server_lib session_lib logger_lib gtest_main gmock_main ${Boost_LIBRARIES})
target_link_libraries(websocket_info_handler_test websocket_info_handler_lib logger_lib gtest_main ${Boost_LIBRARIES})
target_link_libraries(global_websocket_state_test global_websocket_state_lib websocket_handler_lib logger_lib gtest_main ${Boost_LIBRARIES})

add_test(NAME integration_test
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/tests/integration_test.sh
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)

gtest_discover_tests(config_parser_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)
gtest_discover_tests(webserver_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)
gtest_discover_tests(session_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)
gtest_discover_tests(logger_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)
gtest_discover_tests(request_dispatcher_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)
gtest_discover_tests(echo_handler_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)
gtest_discover_tests(static_handler_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)
gtest_discover_tests(serving_config_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)
gtest_discover_tests(not_found_handler_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)
gtest_discover_tests(not_found_handler_factory_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)
gtest_discover_tests(crud_handler_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)
gtest_discover_tests(file_handler_factory_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)
gtest_discover_tests(crud_handler_factory_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)
gtest_discover_tests(crud_filesystem_manager_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)
gtest_discover_tests(health_handler_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)
gtest_discover_tests(websocket_handler_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)
gtest_discover_tests(global_websocket_state_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)

include(cmake/CodeCoverageReportConfig.cmake)
generate_coverage_report(
    TARGETS webserver server_lib session_lib request_dispatcher_lib echo_handler_lib static_handler_lib config_parser_lib 
    logger_lib serving_config_lib static_handler_factory_lib echo_handler_factory_lib not_found_handler_lib 
    not_found_handler_factory_lib file_handler_factory_lib websocket_info_handler_factory_lib crud_handler_lib crud_handler_factory_lib 
    crud_filesystem_manager_lib health_handler_lib websocket_info_handler_lib websocket_handler_lib global_websocket_state_lib
    TESTS config_parser_test webserver_test session_test logger_test request_dispatcher_test echo_handler_test 
    static_handler_test serving_config_test not_found_handler_factory_test websocket_info_handler_factory_test not_found_handler_test crud_handler_test 
    file_handler_factory_test crud_handler_factory_test crud_filesystem_manager_test websocket_info_handler_test health_handler_test websocket_handler_test global_websocket_state_test)

set(COVERAGE_EXCLUDES ${CMAKE_SOURCE_DIR}/tests)
