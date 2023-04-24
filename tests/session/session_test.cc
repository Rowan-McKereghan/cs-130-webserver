#include "session.h"

#include <boost/asio.hpp>
#include <iostream>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "server.h"

class SessionTest : public ::testing::Test {
 protected:
  boost::asio::io_service io_service;
  short port;
  session* sesh;
  boost::system::error_code ec;

  void SetUp() override {
    port = 80;
    sesh = new session(io_service);
  }

  void TearDown() override {}
};

// test steup of session
TEST_F(SessionTest, SessionSetup) {
  sesh->start();
  ASSERT_EQ(sesh->HTTPResponse.substr(0, 16), "HTTP/1.1 200 OK\n");
}

// test correct parsing of end of file
TEST_F(SessionTest, TestEOF) {
  sesh->data_[0] = '\r';
  sesh->data_[1] = '\n';
  sesh->data_[2] = '\r';
  sesh->data_[3] = '\n';
  sesh->handle_read(ec, 4);
  ASSERT_TRUE(sesh->end_of_request);

  sesh->end_of_request = false;
  sesh->data_[0] = '\n';
  sesh->data_[1] = '\n';
  sesh->handle_read(ec, 2);
  ASSERT_TRUE(sesh->end_of_request);
}
// test writing to socket, seeing if handle_write triggers callback
// using mock method for handle_read here to prevent infinite loop
// TEST_F(SessionTest, TestWriteSuccess) {
//   std::string message = "Hello world!\r\n\r\n";
//   using boost::asio::ip::tcp;
//   std::memcpy(sesh->data_, message.c_str(), message.size());

//   // Simulate data on the socket
//   sesh->handle_write(ec);
//   // read callback was executed and end of file was found
//   ASSERT_TRUE(sesh->end_of_request);
// }

// test writing to socket, seeing if handle_write triggers callback
TEST_F(SessionTest, TestWriteSuccess) {
  std::string message = "Hello world!\r\n\r\n";
  using boost::asio::ip::tcp;
  tcp::acceptor acceptor(io_service, tcp::endpoint(tcp::v4(), port));

  acceptor.accept(sesh->socket_);
  acceptor.close();
  // sesh->socket_.connect(tcp::endpoint(tcp::v4(), 80));
  //  Simulate data on the socket
  sesh->handle_write(ec);
  boost::asio::write(sesh->socket_, boost::asio::buffer(message));
  io_service.run_one();
  sesh->socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
  sesh->socket_.close();
  // read callback was executed and end of file was found
  ASSERT_TRUE(sesh->end_of_request);
}
