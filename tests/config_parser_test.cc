#include "gtest/gtest.h"
#include "config_parser.h"

class NginxConfigParserTest : public ::testing::Test {
 protected:
  NginxConfigParser parser;
  NginxConfig out_config;
  short port;
};

TEST_F(NginxConfigParserTest, BasicTests) {
  bool success1 = parser.Parse("BasicTests/basic1", &out_config);
  bool success2 = parser.Parse("BasicTests/basic2", &out_config);
  bool success3 = parser.Parse("BasicTests/empty", &out_config);

  EXPECT_TRUE(success1);
  EXPECT_TRUE(success2);
  EXPECT_TRUE(success3);
}

TEST_F(NginxConfigParserTest, QuotedStrings) {
  bool success1 = parser.Parse("QuotedStrings/success_quote1", &out_config);
  bool success2 = parser.Parse("QuotedStrings/success_quote2", &out_config);
  bool success3 = parser.Parse("QuotedStrings/success_quote3", &out_config);
  bool fail1 = parser.Parse("QuotedStrings/unmatched_quote", &out_config);
  bool fail2 = parser.Parse("QuotedStrings/no_ws_after_quote", &out_config);
  bool fail3 = parser.Parse("QuotedStrings/no_ws_before_quote", &out_config);

  EXPECT_TRUE(success1);
  EXPECT_TRUE(success2);
  EXPECT_TRUE(success3);
  EXPECT_FALSE(fail1);
  EXPECT_FALSE(fail2);
  EXPECT_FALSE(fail3);
}

TEST_F(NginxConfigParserTest, EscapeSeqs) {
  bool success1 = parser.Parse("EscapeSeqs/success_escape1", &out_config);
  bool success2 = parser.Parse("EscapeSeqs/success_escape2", &out_config);
  bool success3 = parser.Parse("EscapeSeqs/success_escape3", &out_config);
  bool fail1 = parser.Parse("EscapeSeqs/mismatch_escape1", &out_config);
  bool fail2 = parser.Parse("EscapeSeqs/esc_double_in_single", &out_config);

  EXPECT_TRUE(success1);
  EXPECT_TRUE(success2);
  EXPECT_TRUE(success3);
  EXPECT_FALSE(fail1);
  EXPECT_FALSE(fail2);
}

TEST_F(NginxConfigParserTest, SuccessiveClosingBrackets) {
  bool success1 = parser.Parse("SuccessiveClosingBrackets/success_brackets1", &out_config);
  bool success2 = parser.Parse("SuccessiveClosingBrackets/success_brackets2", &out_config);
  bool success3 = parser.Parse("SuccessiveClosingBrackets/success_brackets3", &out_config);

  EXPECT_TRUE(success1);
  EXPECT_TRUE(success2);
  EXPECT_TRUE(success3);
}

TEST_F(NginxConfigParserTest, GetPortNumber) {
  bool success1 = parser.GetPortNumber("GetPortNumber/success_pn1", &out_config, &port);
  bool success2 = parser.GetPortNumber("GetPortNumber/success_pn2", &out_config, &port);
  bool fail1 = parser.GetPortNumber("GetPortNumber/no_port_number", &out_config, &port);
  bool fail2 = parser.GetPortNumber("GetPortNumber/no_server_label", &out_config, &port);
  bool fail3 = parser.GetPortNumber("GetPortNumber/nested_too_deep", &out_config, &port);
  bool fail4 = parser.GetPortNumber("GetPortNumber/port_not_number", &out_config, &port);
  bool fail5 = parser.GetPortNumber("GetPortNumber/port_negative", &out_config, &port);

  EXPECT_TRUE(success1);
  EXPECT_TRUE(success2);
  EXPECT_FALSE(fail1);
  EXPECT_FALSE(fail2);
  EXPECT_FALSE(fail3);
  EXPECT_FALSE(fail4);
  EXPECT_FALSE(fail5);
}

