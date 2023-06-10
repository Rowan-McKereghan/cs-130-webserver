#include "global_websocket_state.h"

#include <gtest/gtest.h>

#include "logger.h"
#include "websocket_handler.h"
#include <boost/beast/core.hpp>

class GlobalWebsocketHandlerTest : public ::testing::Test {
 protected:
  GlobalWebsocketState state;
  void SetUp() override {}
};

TEST_F(GlobalWebsocketHandlerTest, TestAdd) {
  std::string channel = "test";
  WebsocketHandler* handler = nullptr;
  state.Add(channel, handler);
  ASSERT_EQ(state.GetChannels().size(), 1);
}

TEST_F(GlobalWebsocketHandlerTest, TestRemove) {
  std::string channel = "test";
  WebsocketHandler* handler = nullptr;
  state.Add(channel, handler);
  ASSERT_EQ(state.GetChannels().size(), 1);
  state.Remove(channel, handler);
}

TEST_F(GlobalWebsocketHandlerTest, Broadcast) {
  std::string channel = "test";
  WebsocketHandler* handler = nullptr;
  std::string msg = "sample message";
  state.Add(channel, handler);
  ASSERT_EQ(state.GetChannels().size(), 1);
  state.Remove(channel, handler);
  boost::beast::flat_buffer msg_cast;
  boost::beast::ostream(msg_cast) << msg;
  state.Broadcast(channel, msg_cast);
}