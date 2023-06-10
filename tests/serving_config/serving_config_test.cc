#include "serving_config.h"

#include <iostream>
#include <cstdlib>

#include "global_websocket_state.h"
#include "websocket_handler.h"
#include "server.h"
#include <memory>

#include "gtest/gtest.h"

class ServingConfigTest : public ::testing::Test {
 protected:
  NginxConfig out_config;
  NginxConfigParser parser;
  ServingConfig serving_config;
  std::shared_ptr<GlobalWebsocketState> state;

  void SetUp() override {
    // Change working directory
    chdir("config_parser");
    state = std::make_shared<GlobalWebsocketState>();
  }
};

// test correct extraction of port number
TEST_F(ServingConfigTest, SetPortNumber) {
  // now need to clear config every time since successful parsing keeps parsed
  // config in pointer, even if getting port number fails

  out_config = NginxConfig();
  EXPECT_TRUE(parser.Parse("../config_parser/GetPortNumber/success_pn1", &out_config) &&
              serving_config.SetPortNumber(&out_config));
  EXPECT_EQ(serving_config.port_, 80);

  out_config = NginxConfig();
  EXPECT_TRUE(parser.Parse("../config_parser/GetPortNumber/success_pn2", &out_config) &&
              serving_config.SetPortNumber(&out_config));
  std::cout << serving_config.port_ << std::endl;
  EXPECT_EQ(serving_config.port_, 800);

  out_config = NginxConfig();
  EXPECT_FALSE(parser.Parse("../config_parser/GetPortNumber/no_port_number", &out_config) &&
               serving_config.SetPortNumber(&out_config));

  out_config = NginxConfig();
  EXPECT_FALSE(parser.Parse("../config_parser/GetPortNumber/no_server_label", &out_config) &&
               serving_config.SetPortNumber(&out_config));

  out_config = NginxConfig();
  EXPECT_FALSE(parser.Parse("../config_parser/GetPortNumber/nested_too_deep", &out_config) &&
               serving_config.SetPortNumber(&out_config));

  out_config = NginxConfig();
  EXPECT_FALSE(parser.Parse("../config_parser/GetPortNumber/port_not_number", &out_config) &&
               serving_config.SetPortNumber(&out_config));

  out_config = NginxConfig();
  EXPECT_FALSE(parser.Parse("../config_parser/GetPortNumber/port_negative", &out_config) &&
               serving_config.SetPortNumber(&out_config));
}

TEST_F(ServingConfigTest, CombinedServingConfig) {
  parser.Parse("GetFilePaths/success_combined", &out_config);
  serving_config.SetPaths(&out_config, state);
  auto hf = serving_config.handler_factories_;
  ASSERT_EQ(hf.size(), 3);
  ASSERT_NE(hf.find("/echoing"), hf.end());
  ASSERT_NE(hf.find("/static3"), hf.end());
}

TEST_F(ServingConfigTest, EchoServingConfig) {
  parser.Parse("GetFilePaths/success_echo_only", &out_config);
  serving_config.SetPaths(&out_config, state);
  auto hf = serving_config.handler_factories_;
  ASSERT_EQ(hf.size(), 2);
  ASSERT_NE(hf.find("/echoing"), hf.end());
}

TEST_F(ServingConfigTest, StaticServingConfig) {
  parser.Parse("GetFilePaths/success_static_path_only", &out_config);
  serving_config.SetPaths(&out_config, state);

  auto hf = serving_config.handler_factories_;
  ASSERT_EQ(hf.size(), 2);
  ASSERT_NE(hf.find("/static3"), hf.end());
}

TEST_F(ServingConfigTest, MultipleUses) {
  parser.Parse("GetFilePaths/multiple_uses", &out_config);
  ASSERT_EQ(serving_config.SetPaths(&out_config, state), 1);
}

TEST_F(ServingConfigTest, InvalidStaticURI) {
  parser.Parse("GetFilePaths/invalid_static_uri", &out_config);
  ASSERT_EQ(serving_config.SetPaths(&out_config, state), 2);
}

TEST_F(ServingConfigTest, InvalidEchoURI) {
  parser.Parse("GetFilePaths/invalid_echo_uri", &out_config);
  ASSERT_EQ(serving_config.SetPaths(&out_config, state), 2);
}