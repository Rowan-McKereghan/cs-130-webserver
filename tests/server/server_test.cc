#include "server.h"

#include <boost/asio.hpp>
#include <boost/filesystem.hpp>
#include <iostream>

#include "I_session.h"
#include "config_parser.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "serving_config.h"

class MockServer : public Server {
 public:
  MockServer(
      boost::asio::io_service& io_service, short port,
      std::function<I_session*(boost::asio::io_service&)> session_constructor)
      : Server(io_service, port, session_constructor) {}
  MOCK_METHOD(void, StartAccept, (), (override));
  MOCK_METHOD(void, HandleAccept,
              (I_session * new_session, const boost::system::error_code& error),
              (override));
};

class MockSession : public I_session {
 public:
  MockSession(boost::asio::io_service& io_service, ServingConfig serving_config)
      : socket_(io_service), serving_config_(serving_config) {}
  MOCK_METHOD(boost::asio::ip::tcp::socket&, get_socket, (), (override));
  MOCK_METHOD(void, Start, (), (override));

 private:
  boost::asio::ip::tcp::socket socket_;
  ServingConfig serving_config_;
};

class ServerTest : public ::testing::Test {
 protected:
  boost::asio::io_service io_service;
  short port;
  Server* server_;
  boost::system::error_code ec;

  void SetUp() override {
    port = 80;
    ServingConfig serving_config;
    // Default root path (for now, testing purposes only)
    serving_config.static_file_paths_ = {{"static1", "/usr/src/projects/"}};
    server_ = new Server(io_service, port,
                         [serving_config](boost::asio::io_service& io_service) {
                           return new MockSession(io_service, serving_config);
                         });
  }

  void TearDown() override { delete server_; }
};

TEST_F(ServerTest, ServerSetup) {
  boost::asio::ip::tcp::socket sock = boost::asio::ip::tcp::socket(io_service);
  testing::DefaultValue<boost::asio::ip::tcp::socket&>::Set(sock);

  // set default value fo MockSession->socket() on initial run
  server_->StartAccept();
  MockSession* session1 =
      dynamic_cast<MockSession*>(server_->get_cur_session());

  // becuase second call has error, Start() should only be called once
  EXPECT_CALL(*session1, Start()).Times(1);
  server_->HandleAccept(session1, ec);
  ec.assign(123, boost::system::generic_category());
  server_->HandleAccept(session1, ec);

  server_->StartAccept();
  MockSession* session2 =
      dynamic_cast<MockSession*>(server_->get_cur_session());
  // accepting new session should have changed active session
  ASSERT_NE(session1, session2);

  delete session1;
  delete session2;
}
