#include "config_parser.h"

#include <iostream>

#include "gtest/gtest.h"
#include "serving_config.h"

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
  EXPECT_TRUE(parser.Parse("SuccessiveClosingBrackets/success_brackets1", &out_config));
  EXPECT_TRUE(parser.Parse("SuccessiveClosingBrackets/success_brackets2", &out_config));
  EXPECT_TRUE(parser.Parse("SuccessiveClosingBrackets/success_brackets3", &out_config));
}