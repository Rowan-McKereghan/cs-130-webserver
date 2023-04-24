#include "server.h"

#include <boost/asio.hpp>
#include <iostream>

#include "I_session.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

class MockServer : public server {
 public:
  MockServer(
      boost::asio::io_service& io_service, short port,
      std::function<I_session*(boost::asio::io_service&)> session_constructor)
      : server(io_service, port, session_constructor) {}
  MOCK_METHOD(void, start_accept, (), (override));
  MOCK_METHOD(void, handle_accept,
              (I_session * new_session, const boost::system::error_code& error),
              (override));
};

class MockSession : public I_session {
 public:
  MockSession(boost::asio::io_service& io_service) : socket_(io_service) {}
  MOCK_METHOD(boost::asio::ip::tcp::socket&, socket, (), (override));
  MOCK_METHOD(void, start, (), (override));

 private:
  boost::asio::ip::tcp::socket socket_;
};

class ServerTest : public ::testing::Test {
 protected:
  boost::asio::io_service io_service;
  short port;
  server* server_;
  boost::system::error_code ec;

  void SetUp() override {
    port = 80;
    server_ =
        new server(io_service, port, [](boost::asio::io_service& io_service) {
          return new MockSession(io_service);
        });
  }

  void TearDown() override { delete server_; }
};

TEST_F(ServerTest, ServerSetup) {
  boost::asio::ip::tcp::socket sock = boost::asio::ip::tcp::socket(io_service);
  testing::DefaultValue<boost::asio::ip::tcp::socket&>::Set(sock);

  // set default value fo MockSession->socket() on initial run
  server_->start_accept();
  MockSession* session1 =
      dynamic_cast<MockSession*>(server_->get_cur_session());

  // becuase second call has error, start() should only be called once
  EXPECT_CALL(*session1, start()).Times(1);
  server_->handle_accept(session1, ec);
  ec.assign(123, boost::system::generic_category());
  server_->handle_accept(session1, ec);

  server_->start_accept();
  MockSession* session2 =
      dynamic_cast<MockSession*>(server_->get_cur_session());
  // accepting new session should have changed active session
  ASSERT_NE(session1, session2);

  delete session1;
  delete session2;
}
