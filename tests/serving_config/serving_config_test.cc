#include "serving_config.h"

#include <iostream>

#include "gtest/gtest.h"

class ServingConfigTest : public ::testing::Test {
 protected:
  NginxConfig out_config;
  NginxConfigParser parser;
  ServingConfig serving_config;

  void SetUp() override {
    // Change working directory
    chdir("config_parser");
  }
};

// test correct extraction of port number
TEST_F(ServingConfigTest, SetPortNumber) {
  // now need to clear config every time since successful parsing keeps parsed
  // config in pointer, even if getting port number fails

  out_config = NginxConfig();
  EXPECT_TRUE(
      parser.Parse("../config_parser/GetPortNumber/success_pn1", &out_config) &&
      serving_config.set_port_number(&out_config));
  EXPECT_EQ(serving_config.port_, 80);

  out_config = NginxConfig();
  EXPECT_TRUE(
      parser.Parse("../config_parser/GetPortNumber/success_pn2", &out_config) &&
      serving_config.set_port_number(&out_config));
  std::cout << serving_config.port_ << std::endl;
  EXPECT_EQ(serving_config.port_, 800);

  out_config = NginxConfig();
  EXPECT_FALSE(parser.Parse("../config_parser/GetPortNumber/no_port_number",
                            &out_config) &&
               serving_config.set_port_number(&out_config));

  out_config = NginxConfig();
  EXPECT_FALSE(parser.Parse("../config_parser/GetPortNumber/no_server_label",
                            &out_config) &&
               serving_config.set_port_number(&out_config));

  out_config = NginxConfig();
  EXPECT_FALSE(parser.Parse("../config_parser/GetPortNumber/nested_too_deep",
                            &out_config) &&
               serving_config.set_port_number(&out_config));

  out_config = NginxConfig();
  EXPECT_FALSE(parser.Parse("../config_parser/GetPortNumber/port_not_number",
                            &out_config) &&
               serving_config.set_port_number(&out_config));

  out_config = NginxConfig();
  EXPECT_FALSE(parser.Parse("../config_parser/GetPortNumber/port_negative",
                            &out_config) &&
               serving_config.set_port_number(&out_config));
}

TEST_F(ServingConfigTest, CombinedServingConfig) {
  parser.Parse("GetFilePaths/success_combined", &out_config);
  serving_config.set_paths(&out_config);
  ASSERT_EQ(serving_config.echo_paths_.size(), 1);
  ASSERT_EQ(serving_config.echo_paths_[0], "/echoing");
  ASSERT_EQ(serving_config.static_file_paths_.size(), 1);
  ASSERT_EQ(serving_config.static_file_paths_[0].first, "/static3");
  ASSERT_EQ(serving_config.static_file_paths_[0].second, "../static_test_files");
}

TEST_F(ServingConfigTest, EchoServingConfig) {
  parser.Parse("GetFilePaths/success_echo_only", &out_config);
  serving_config.set_paths(&out_config);
  ASSERT_EQ(serving_config.echo_paths_.size(), 1);
  ASSERT_EQ(serving_config.echo_paths_[0], "/echoing");
}

TEST_F(ServingConfigTest, StaticServingConfig) {
  parser.Parse("GetFilePaths/success_static_path_only", &out_config);
  serving_config.set_paths(&out_config);

  ASSERT_EQ(serving_config.echo_paths_.size(), 0);
  ASSERT_EQ(serving_config.static_file_paths_.size(), 1);
  ASSERT_EQ(serving_config.static_file_paths_[0].first, "/static3");
  ASSERT_EQ(serving_config.static_file_paths_[0].second, "../static_test_files");
}

TEST_F(ServingConfigTest, PartialSuccessServingConfig) {
  parser.Parse("GetFilePaths/partial_success", &out_config);
  serving_config.set_paths(&out_config);
  ASSERT_EQ(serving_config.echo_paths_.size(), 1);
  ASSERT_EQ(serving_config.echo_paths_[0], "/potato");
  ASSERT_EQ(serving_config.static_file_paths_.size(), 1);
  ASSERT_EQ(serving_config.static_file_paths_[0].first, "/static3");
  ASSERT_EQ(serving_config.static_file_paths_[0].second, "../static_test_files");
}

TEST_F(ServingConfigTest, PathLengthsSorted) {
  parser.Parse("GetFilePaths/many_paths", &out_config);
  serving_config.set_paths(&out_config);
  ASSERT_EQ(serving_config.static_file_paths_.size(), 6);
  ASSERT_EQ(serving_config.static_file_paths_[0].first, "/static4/sub1/sub2");
  ASSERT_EQ(serving_config.static_file_paths_[1].first, "/static1/static2");
  ASSERT_EQ(serving_config.static_file_paths_[2].first, "/static3/sub");
  ASSERT_EQ(serving_config.static_file_paths_[3].first, "/static4/sub1");
  ASSERT_EQ(serving_config.static_file_paths_[4].first, "/static3");
  ASSERT_EQ(serving_config.static_file_paths_[5].first, "/static4");
}

TEST_F(ServingConfigTest, PrivilegedPaths) {
  parser.Parse("GetFilePaths/privileged_paths", &out_config);
  serving_config.set_paths(&out_config);
  ASSERT_EQ(serving_config.static_file_paths_.size(), 1);
  ASSERT_EQ(serving_config.static_file_paths_[0].first, "/static3");
}