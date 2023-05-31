#include "server.h"

#include <boost/asio.hpp>
#include <boost/filesystem.hpp>
#include <iostream>

#include "I_session.h"
#include "config_parser.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "serving_config.h"
#include "session.h"

class MockServer : public Server {
 public:
  MockServer(boost::asio::io_service& io_service, short port,
             std::function<std::shared_ptr<I_session>(boost::asio::io_service&)> session_constructor)
      : Server(io_service, port, session_constructor) {}
  MOCK_METHOD(void, StartSession, (std::shared_ptr<I_session> new_session), (override));
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
  MockServer* server_;
  boost::system::error_code ec;

  void SetUp() override {
    port = 80;

    ServingConfig serving_config;
    // Default root path (for now, testing purposes only)
    server_ = new MockServer(io_service, port, [serving_config](boost::asio::io_service& io_service) {
      return std::make_shared<Session>(io_service, serving_config, std::make_shared<GlobalWebsocketState>());
    });
  }

  void TearDown() override { delete server_; }
};

TEST_F(ServerTest, ServerSetup) {
  boost::asio::ip::tcp::socket sock = boost::asio::ip::tcp::socket(io_service);
  testing::DefaultValue<boost::asio::ip::tcp::socket&>::Set(sock);

  // set default value fo MockSession->socket() on initial run
  server_->StartAccept();
  std::shared_ptr<I_session> session1 = server_->get_cur_session();

  // becuase second HandleAccept call has an error, StartSession should only be called once
  EXPECT_CALL(*server_, StartSession(session1)).Times(1);
  server_->HandleAccept(session1, ec);
  ec.assign(123, boost::system::generic_category());
  server_->HandleAccept(session1, ec);

  server_->StartAccept();
  std::shared_ptr<I_session> session2 = server_->get_cur_session();
  // accepting new session should have changed active session
  ASSERT_NE(session1, session2);
}
