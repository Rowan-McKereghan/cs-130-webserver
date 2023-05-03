#include "config_parser.h"

#include <iostream>

#include "gtest/gtest.h"

class NginxConfigParserTest : public ::testing::Test {
 protected:
  NginxConfigParser parser;
  NginxConfig out_config;
  ServingConfig serving_config;

  short port;

  void SetUp() override {
    // Change working directory
    chdir("config_parser");
  }
};

// basic tests with no special behavior
TEST_F(NginxConfigParserTest, BasicTests) {
  EXPECT_TRUE(parser.Parse("BasicTests/basic1", &out_config));
  EXPECT_TRUE(parser.Parse("BasicTests/basic2", &out_config));
  EXPECT_TRUE(parser.Parse("BasicTests/empty", &out_config));
}

// test configs with comments
TEST_F(NginxConfigParserTest, CommentTests) {
  EXPECT_TRUE(parser.Parse("CommentTests/comment1", &out_config));
  EXPECT_FALSE(parser.Parse("CommentTests/comment_hides_token", &out_config));
}

// test given ToString functions
TEST_F(NginxConfigParserTest, ToStringTests) {
  parser.Parse("BasicTests/basic1", &out_config);
  ASSERT_EQ(out_config.ToString(0)[10], '\n');
  ASSERT_EQ(out_config.ToString(1)[12], '\n');
}

// test that parser enforces whitespace in cases of
// normal token and quoted string and two quoted strings
TEST_F(NginxConfigParserTest, QuotedStrings) {
  EXPECT_TRUE(parser.Parse("QuotedStrings/success_quote1", &out_config));
  EXPECT_TRUE(parser.Parse("QuotedStrings/success_quote2", &out_config));
  EXPECT_TRUE(parser.Parse("QuotedStrings/success_quote3", &out_config));
  EXPECT_FALSE(parser.Parse("QuotedStrings/unmatched_quote", &out_config));
  EXPECT_FALSE(parser.Parse("QuotedStrings/no_ws_after_quote", &out_config));
  EXPECT_FALSE(parser.Parse("QuotedStrings/no_ws_before_quote", &out_config));
}

// test that escape sequences are escaped properly within quoted strings
TEST_F(NginxConfigParserTest, EscapeSeqs) {
  EXPECT_TRUE(parser.Parse("EscapeSeqs/success_escape1", &out_config));
  EXPECT_TRUE(parser.Parse("EscapeSeqs/success_escape2", &out_config));
  EXPECT_TRUE(parser.Parse("EscapeSeqs/success_escape3", &out_config));
  EXPECT_FALSE(parser.Parse("EscapeSeqs/mismatch_escape1", &out_config));
  EXPECT_FALSE(parser.Parse("EscapeSeqs/esc_double_in_single", &out_config));
}

// test that successive closing brackets are able to follow each other if
// bracket balance is maintained
TEST_F(NginxConfigParserTest, SuccessiveClosingBrackets) {
  EXPECT_TRUE(
      parser.Parse("SuccessiveClosingBrackets/success_brackets1", &out_config));
  EXPECT_TRUE(
      parser.Parse("SuccessiveClosingBrackets/success_brackets2", &out_config));
  EXPECT_TRUE(
      parser.Parse("SuccessiveClosingBrackets/success_brackets3", &out_config));
}

// test correct extraction of port number
TEST_F(NginxConfigParserTest, GetPortNumber) {
  // now need to clear config every time since successful parsing keeps parsed
  // config in pointer, even if getting port number fails
  EXPECT_TRUE(parser.Parse("GetPortNumber/success_pn1", &out_config) &&
              parser.GetPortNumber(&out_config, &port));
  out_config = NginxConfig();
  EXPECT_TRUE(parser.Parse("GetPortNumber/success_pn2", &out_config) &&
              parser.GetPortNumber(&out_config, &port));
  out_config = NginxConfig();
  EXPECT_FALSE(parser.Parse("GetPortNumber/no_port_number", &out_config) &&
               parser.GetPortNumber(&out_config, &port));
  out_config = NginxConfig();
  EXPECT_FALSE(parser.Parse("GetPortNumber/no_server_label", &out_config) &&
               parser.GetPortNumber(&out_config, &port));
  out_config = NginxConfig();
  EXPECT_FALSE(parser.Parse("GetPortNumber/nested_too_deep", &out_config) &&
               parser.GetPortNumber(&out_config, &port));
  out_config = NginxConfig();
  EXPECT_FALSE(parser.Parse("GetPortNumber/port_not_number", &out_config) &&
               parser.GetPortNumber(&out_config, &port));
  out_config = NginxConfig();
  EXPECT_FALSE(parser.Parse("GetPortNumber/port_negative", &out_config) &&
               parser.GetPortNumber(&out_config, &port));
}

// Tests for IsValidURI method
TEST_F(NginxConfigParserTest, ValidURI) {
  EXPECT_TRUE(parser.IsValidURI("/path/to/resource"));
  EXPECT_TRUE(parser.IsValidURI("/path/to/resource.ext"));
  EXPECT_TRUE(parser.IsValidURI("/path/to/resource-with-dash"));
  EXPECT_TRUE(parser.IsValidURI(
      "/path/to/resource_with_underscore_and_trailing_slash/"));
}

TEST_F(NginxConfigParserTest, InvalidURI) {
  EXPECT_FALSE(parser.IsValidURI("path/to/resource"));
  EXPECT_FALSE(parser.IsValidURI("/path/to/resource?query=param"));
  EXPECT_FALSE(parser.IsValidURI("/path/to/resource#hashtag"));
}

// Tests for VerifyServerConfig method
TEST_F(NginxConfigParserTest, EmptyConfig) {
  EXPECT_FALSE(parser.VerifyServerConfig(serving_config));
}

TEST_F(NginxConfigParserTest, InvalidStaticFilePath) {
  serving_config.static_file_paths.push_back(
      {"path/to/resource", "/nonexistent/file"});
  EXPECT_FALSE(parser.VerifyServerConfig(serving_config));
}

TEST_F(NginxConfigParserTest, ValidStaticFilePath) {
  // Relative to tests/config_parser
  serving_config.static_file_paths.push_back(
      {"/path/to/resource", "../static_test_files"});
  EXPECT_TRUE(parser.VerifyServerConfig(serving_config));
}

TEST_F(NginxConfigParserTest, InvalidEchoPath) {
  serving_config.echo_paths.push_back("/path/to/resource#hash");
  EXPECT_FALSE(parser.VerifyServerConfig(serving_config));
}

TEST_F(NginxConfigParserTest, ValidEchoPath) {
  serving_config.echo_paths.push_back("/path/to/resource");
  EXPECT_TRUE(parser.VerifyServerConfig(serving_config));
}

TEST_F(NginxConfigParserTest, EmptyServingConfig) {
  parser.Parse("BasicTests/empty", &out_config);
  parser.GetServingConfig(&out_config, serving_config);
  ASSERT_EQ(serving_config.echo_paths.size(), 0);
  ASSERT_EQ(serving_config.static_file_paths.size(), 0);
}

TEST_F(NginxConfigParserTest, CombinedServingConfig) {
  parser.Parse("GetFilePaths/success_combined", &out_config);
  parser.GetServingConfig(&out_config, serving_config);
  ASSERT_EQ(serving_config.echo_paths.size(), 1);
  ASSERT_EQ(serving_config.echo_paths[0], "/echoing");
  ASSERT_EQ(serving_config.static_file_paths.size(), 1);
  ASSERT_EQ(serving_config.static_file_paths[0].first, "/static3");
  ASSERT_EQ(serving_config.static_file_paths[0].second, "../static_test_files");
}

TEST_F(NginxConfigParserTest, EchoServingConfig) {
  parser.Parse("GetFilePaths/success_echo_only", &out_config);
  parser.GetServingConfig(&out_config, serving_config);
  ASSERT_EQ(serving_config.echo_paths.size(), 1);
  ASSERT_EQ(serving_config.echo_paths[0], "/echoing");
}
TEST_F(NginxConfigParserTest, StaticServingConfig) {
  parser.Parse("GetFilePaths/success_static_path_only", &out_config);
  parser.GetServingConfig(&out_config, serving_config);

  ASSERT_EQ(serving_config.echo_paths.size(), 0);
  ASSERT_EQ(serving_config.static_file_paths.size(), 1);
  ASSERT_EQ(serving_config.static_file_paths[0].first, "/static3");
  ASSERT_EQ(serving_config.static_file_paths[0].second, "../static_test_files");
}

TEST_F(NginxConfigParserTest, PartialSuccessServingConfig) {
  parser.Parse("GetFilePaths/partial_success", &out_config);
  parser.GetServingConfig(&out_config, serving_config);
  ASSERT_EQ(serving_config.echo_paths.size(), 1);
  ASSERT_EQ(serving_config.echo_paths[0], "/potato");
  ASSERT_EQ(serving_config.static_file_paths.size(), 1);
  ASSERT_EQ(serving_config.static_file_paths[0].first, "/static3");
  ASSERT_EQ(serving_config.static_file_paths[0].second, "../static_test_files");
}