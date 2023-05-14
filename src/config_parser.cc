// An nginx config file parser.
//
// See:
//   http://wiki.nginx.org/Configuration
//   http://blog.martinfjordvald.com/2010/07/nginx-primer/
//
// How Nginx does it:
//   http://lxr.nginx.org/source/src/core/ngx_conf_file.c

#include "config_parser.h"

#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/regex.hpp>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <memory>
#include <stack>
#include <string>
#include <vector>

#include "logger.h"

// helper function headers
std::string ProcessEsc(std::string token);
bool GetPortNumberHelper(const NginxConfig* config, short* port);

std::string NginxConfig::ToString(int depth) {
  std::string serialized_config;
  for (const auto& statement : statements_) {
    serialized_config.append(statement->ToString(depth));
  }
  return serialized_config;
}

std::string NginxConfigStatement::ToString(int depth) {
  std::string serialized_statement;
  for (int i = 0; i < depth; ++i) {
    serialized_statement.append("  ");
  }
  for (unsigned int i = 0; i < tokens_.size(); ++i) {
    if (i != 0) {
      serialized_statement.append(" ");
    }
    serialized_statement.append(tokens_[i]);
  }
  if (child_block_.get() != nullptr) {
    serialized_statement.append(" {\n");
    serialized_statement.append(child_block_->ToString(depth + 1));
    for (int i = 0; i < depth; ++i) {
      serialized_statement.append("  ");
    }
    serialized_statement.append("}");
  } else {
    serialized_statement.append(";");
  }
  serialized_statement.append("\n");
  return serialized_statement;
}

const char* NginxConfigParser::TokenTypeAsString(TokenType type) {
  switch (type) {
    case kTokenTypeStart:
      return "kTokenTypeStart";
    case kTokenTypeNormal:
      return "kTokenTypeNormal";
    case kTokenTypeStartBlock:
      return "kTokenTypeStartBlock";
    case kTokenTypeEndBlock:
      return "kTokenTypeEndBlock";
    case kTokenTypeComment:
      return "kTokenTypeComment";
    case kTokenTypeStatementEnd:
      return "kTokenTypeStatementEnd";
    case kTokenTypeEOF:
      return "kTokenTypeEOF";
    case kTokenTypeError:
      return "kTokenTypeError";
    case kTokenTypeQuotedString:
      return "kTokenTypeQuotedString";  // represents completed quote
    default:
      return "Unknown token type";
  }
}

NginxConfigParser::TokenType NginxConfigParser::ParseToken(
    std::istream* input, std::string* value, bool& seenWhitespace) {
  TokenParserState state = kTokenStateInitialWhitespace;
  bool inEscapeSeq = false;
  while (input->good()) {
    const char c = input->get();
    if (!input->good()) {
      break;
    }
    switch (state) {
      case kTokenStateInitialWhitespace:
        switch (c) {
          case '{':
            *value = c;
            return kTokenTypeStartBlock;
          case '}':
            *value = c;
            return kTokenTypeEndBlock;
          case '#':
            *value = c;
            state = kTokenStateTokenTypeComment;
            continue;
          case '"':
            *value = c;
            state = kTokenStateDoubleQuote;
            continue;
          case '\'':
            *value = c;
            state = kTokenStateSingleQuote;
            continue;
          case ';':
            *value = c;
            return kTokenTypeStatementEnd;
          case ' ':
            seenWhitespace = true;
          case '\t':
          case '\n':
          case '\r':
            continue;
          default:
            *value += c;
            if (state != kTokenStateDoubleQuote &&
                state != kTokenStateSingleQuote)
              state = kTokenStateTokenTypeNormal;
            continue;
        }
      // logic for handling escape sequences
      case kTokenStateSingleQuote:
        *value += c;
        if (inEscapeSeq) {
          if (c != '\\' && c != '\'') return kTokenTypeError;
          inEscapeSeq = false;
          continue;
        }
        if (c == '\\' && !inEscapeSeq) {
          inEscapeSeq = true;
          continue;
        }
        if (c == '\'' && !inEscapeSeq) {
          // we obtain the entire unescaped string and then
          // process escape sequences
          *value = ProcessEsc(*value);
          return kTokenTypeQuotedString;
        }
        continue;
      case kTokenStateDoubleQuote:
        *value += c;
        if (inEscapeSeq) {
          if (c != '\\' && c != '"') return kTokenTypeError;
          inEscapeSeq = false;
          continue;
        }
        if (c == '\\' && !inEscapeSeq) {
          inEscapeSeq = true;
          continue;
        }
        if (c == '"' && !inEscapeSeq) {
          *value = ProcessEsc(*value);
          return kTokenTypeQuotedString;
        }
        continue;
      case kTokenStateTokenTypeComment:
        if (c == '\n' || c == '\r') {
          return kTokenTypeComment;
        }
        *value += c;
        continue;
      case kTokenStateTokenTypeNormal:
        if (c == ' ' || c == '\t' || c == '\n' || c == '\t' || c == ';' ||
            c == '{' || c == '}' || c == '\'' || c == '"') {
          input->unget();
          return kTokenTypeNormal;
        }
        *value += c;
        continue;
    }
  }

  // If we get here, we reached the end of the file.
  if (state == kTokenStateSingleQuote || state == kTokenStateDoubleQuote) {
    return kTokenTypeError;
  }

  return kTokenTypeEOF;
}

bool NginxConfigParser::Parse(std::istream* config_file, NginxConfig* config) {
  std::stack<NginxConfig*> config_stack;
  config_stack.push(config);
  TokenType last_token_type = kTokenTypeStart;
  TokenType token_type;
  int error = 0;
  int bracket_balance = 0;
  while (true) {
    std::string token;
    bool seenWhitespace =
        false;  // flag set to true whenever whitespace encountered
    token_type = ParseToken(config_file, &token, seenWhitespace);
    if (token_type == kTokenTypeError) {
      break;
    }

    if (token_type == kTokenTypeComment) {
      // Skip comments.
      continue;
    }

    if (token_type == kTokenTypeStart) {
      // Error.
      break;
    } else if (token_type == kTokenTypeNormal ||
               token_type == kTokenTypeQuotedString) {
      if (last_token_type == kTokenTypeStart ||
          last_token_type == kTokenTypeStatementEnd ||
          last_token_type == kTokenTypeStartBlock ||
          last_token_type == kTokenTypeEndBlock ||
          // enforce space in case of normal before quote and quote before quote
          seenWhitespace &&
              (token_type == kTokenTypeNormal ||
               token_type == kTokenTypeQuotedString) &&
              (last_token_type == kTokenTypeNormal ||
               last_token_type == kTokenTypeQuotedString)) {
        if (last_token_type != kTokenTypeNormal &&
            last_token_type != kTokenTypeQuotedString) {
          config_stack.top()->statements_.emplace_back(
              new NginxConfigStatement);
        }
        config_stack.top()->statements_.back().get()->tokens_.push_back(token);
      } else {
        if (!seenWhitespace &&
            (token_type == kTokenTypeNormal &&
                 last_token_type == kTokenTypeQuotedString ||
             token_type == kTokenTypeQuotedString &&
                 last_token_type == kTokenTypeNormal))
          error = 1;
        // Error.
        break;
      }
    } else if (token_type == kTokenTypeStatementEnd) {
      if (last_token_type != kTokenTypeNormal &&
          last_token_type != kTokenTypeQuotedString) {
        // Error.
        break;
      }
    } else if (token_type == kTokenTypeStartBlock) {
      bracket_balance++;
      if (last_token_type != kTokenTypeNormal &&
          last_token_type != kTokenTypeQuotedString) {
        // Error.
        break;
      }
      NginxConfig* const new_config = new NginxConfig;
      config_stack.top()->statements_.back().get()->child_block_.reset(
          new_config);
      config_stack.push(new_config);
    } else if (token_type == kTokenTypeEndBlock) {
      bracket_balance--;
      if (last_token_type != kTokenTypeStatementEnd &&
          last_token_type != kTokenTypeStartBlock && 
          last_token_type != kTokenTypeEndBlock) {
        // Error.
        break;
      }
      config_stack.pop();
    } else if (token_type == kTokenTypeEOF) {
      if (last_token_type != kTokenTypeStatementEnd &&
          last_token_type != kTokenTypeEndBlock &&
          last_token_type != kTokenTypeStart) {
        // Error.
        break;
      }
      if (bracket_balance != 0) break;
      if (bracket_balance != 0) break;
      return true;
    } else {
      // Error. Unknown token.
      break;
    }
    last_token_type = token_type;
  }
  if (error == 1) {
    LOG(error) << "No whitespace between " << TokenTypeAsString(last_token_type)
               << " and " << TokenTypeAsString(token_type);
  } else {
    LOG(error) << "Bad transition from " << TokenTypeAsString(last_token_type)
               << " to " << TokenTypeAsString(token_type);
  }
  return false;
}

bool NginxConfigParser::Parse(const char* file_name, NginxConfig* config) {
  std::ifstream config_file;
  config_file.open(file_name);
  if (!config_file.good()) {
    LOG(error) << "Failed to open config file" << file_name;
    return false;
  }

  const bool return_value =
      Parse(dynamic_cast<std::istream*>(&config_file), config);
  config_file.close();
  return return_value;
}

// helper function for converting escape sequences into the chars they represent
// only support for '\\', '\'' inside "", and '"' inside ''
std::string ProcessEsc(std::string token) {
  std::string processed_token = "";
  bool inEscapeSeq = false;
  for (auto c : token) {
    if (c == '\\' && !inEscapeSeq) {
      inEscapeSeq = true;
      continue;
    }
    if (inEscapeSeq) inEscapeSeq = false;
    processed_token.push_back(c);
  }
  return processed_token;
}