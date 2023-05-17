#ifndef PARSER_H
#define PARSER_H

// An nginx config file parser.

#include <iostream>
#include <memory>
#include <string>
#include <vector>

class NginxConfig;

// The parsed representation of a single config statement.
class NginxConfigStatement {
 public:
  std::string ToString(int depth);
  std::vector<std::string> tokens_;
  std::unique_ptr<NginxConfig> child_block_;
};

// The parsed representation of the entire config.
class NginxConfig {
 public:
  std::string ToString(int depth = 0);
  std::vector<std::shared_ptr<NginxConfigStatement>> statements_;
};

// The driver that parses a config file and generates an NginxConfig.
class NginxConfigParser {
 public:
  NginxConfigParser() {}

  // Take a opened config file or file name (respectively) and store the
  // parsed config in the provided NginxConfig out-param.  Returns true
  // iff the input config file is valid.
  bool Parse(std::istream* config_file, NginxConfig* config);
  bool Parse(const char* file_name, NginxConfig* config);

 private:
  enum TokenType {
    kTokenTypeStart = 0,
    kTokenTypeNormal = 1,
    kTokenTypeStartBlock = 2,
    kTokenTypeEndBlock = 3,
    kTokenTypeComment = 4,
    kTokenTypeStatementEnd = 5,
    kTokenTypeEOF = 6,
    kTokenTypeError = 7,
    kTokenTypeQuotedString = 8
  };
  const char* TokenTypeAsString(TokenType type);

  enum TokenParserState {
    kTokenStateInitialWhitespace = 0,
    kTokenStateSingleQuote = 1,
    kTokenStateDoubleQuote = 2,
    kTokenStateTokenTypeComment = 3,
    kTokenStateTokenTypeNormal = 4
  };

  TokenType ParseToken(std::istream* input, std::string* value, bool& seenWhitespace);
};

#endif