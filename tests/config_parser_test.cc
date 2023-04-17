#include "gtest/gtest.h"
#include "config_parser.h"

class NginxConfigParserTest : public ::testing::Test {
 protected:
  NginxConfigParser parser;
  NginxConfig out_config;
  short port;
};

// baisc tests with no special behavior
TEST_F(NginxConfigParserTest, BasicTests) {
  EXPECT_TRUE(parser.Parse("BasicTests/basic1", &out_config));
  EXPECT_TRUE(parser.Parse("BasicTests/basic2", &out_config));
  EXPECT_TRUE(parser.Parse("BasicTests/empty", &out_config));
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


// test correct extraction of port number
TEST_F(NginxConfigParserTest, GetPortNumber) {
  // now need to clear config every time since successful parsing keeps parsed 
  // config in pointer, even if getting port number fails
  EXPECT_TRUE(parser.Parse("GetPortNumber/success_pn1", &out_config) && parser.GetPortNumber(&out_config, &port));
  out_config = NginxConfig();
  EXPECT_TRUE(parser.Parse("GetPortNumber/success_pn2", &out_config) && parser.GetPortNumber(&out_config, &port));
  out_config = NginxConfig();
  EXPECT_FALSE(parser.Parse("GetPortNumber/no_port_number", &out_config) && parser.GetPortNumber(&out_config, &port));
  out_config = NginxConfig();
  EXPECT_FALSE(parser.Parse("GetPortNumber/no_server_label", &out_config) && parser.GetPortNumber(&out_config, &port));
  out_config = NginxConfig();
  EXPECT_FALSE(parser.Parse("GetPortNumber/nested_too_deep", &out_config) && parser.GetPortNumber(&out_config, &port));
  out_config = NginxConfig();
  EXPECT_FALSE(parser.Parse("GetPortNumber/port_not_number", &out_config) && parser.GetPortNumber(&out_config, &port));
  out_config = NginxConfig();
  EXPECT_FALSE(parser.Parse("GetPortNumber/port_negative", &out_config) && parser.GetPortNumber(&out_config, &port));
}

