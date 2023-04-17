// An nginx config file parser.
//
// See:
//   http://wiki.nginx.org/Configuration
//   http://blog.martinfjordvald.com/2010/07/nginx-primer/
//
// How Nginx does it:
//   http://lxr.nginx.org/source/src/core/ngx_conf_file.c

#include <cstdio>
#include <fstream>
#include <iostream>
#include <memory>
#include <stack>
#include <string>
#include <vector>
#include "config_parser.h"

// helper function headers
std::string process_esc(std::string token);
bool is_number(const std::string& s);
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
    case TOKEN_TYPE_START:         return "TOKEN_TYPE_START";
    case TOKEN_TYPE_NORMAL:        return "TOKEN_TYPE_NORMAL";
    case TOKEN_TYPE_START_BLOCK:   return "TOKEN_TYPE_START_BLOCK";
    case TOKEN_TYPE_END_BLOCK:     return "TOKEN_TYPE_END_BLOCK";
    case TOKEN_TYPE_COMMENT:       return "TOKEN_TYPE_COMMENT";
    case TOKEN_TYPE_STATEMENT_END: return "TOKEN_TYPE_STATEMENT_END";
    case TOKEN_TYPE_EOF:           return "TOKEN_TYPE_EOF";
    case TOKEN_TYPE_ERROR:         return "TOKEN_TYPE_ERROR";
    case TOKEN_TYPE_QUOTED_STRING: return "TOKEN_TYPE_QUOTED_STRING"; // represents completed quote
    default:                       return "Unknown token type";
  }
}

NginxConfigParser::TokenType NginxConfigParser::ParseToken(std::istream* input,
                                                           std::string* value, bool& seenWhitespace) {
  TokenParserState state = TOKEN_STATE_INITIAL_WHITESPACE;
  bool inEscapeSeq = false;
  while (input->good()) {
    const char c = input->get();
    if (!input->good()) {
      break;
    }
    switch (state) {
      case TOKEN_STATE_INITIAL_WHITESPACE:
        switch (c) {
          case '{':
            *value = c;
            return TOKEN_TYPE_START_BLOCK;
          case '}':
            *value = c;
            return TOKEN_TYPE_END_BLOCK;
          case '#':
            *value = c;
            state = TOKEN_STATE_TOKEN_TYPE_COMMENT;
            continue;
          case '"':
            *value = c;
            state = TOKEN_STATE_DOUBLE_QUOTE;
            continue;
          case '\'':
            *value = c;
            state = TOKEN_STATE_SINGLE_QUOTE;
            continue;
          case ';':
            *value = c;
            return TOKEN_TYPE_STATEMENT_END;
          case ' ':
            seenWhitespace = true;
          case '\t':
          case '\n':
          case '\r':
            continue;
          default:
            *value += c;
            if (state != TOKEN_STATE_DOUBLE_QUOTE && state != TOKEN_STATE_SINGLE_QUOTE)
              state = TOKEN_STATE_TOKEN_TYPE_NORMAL;
            continue;
        }
      // logic for handling escape sequences
      case TOKEN_STATE_SINGLE_QUOTE:
        *value += c;
        if (inEscapeSeq) {
          if (c != '\\' && c != '\'')
            return TOKEN_TYPE_ERROR;
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
          *value = process_esc(*value);
          return TOKEN_TYPE_QUOTED_STRING;
        }
        continue;
      case TOKEN_STATE_DOUBLE_QUOTE:
        *value += c;
        if (inEscapeSeq) {
          if (c != '\\' && c != '"')
            return TOKEN_TYPE_ERROR;
          inEscapeSeq = false;
          continue;
        }
        if (c == '\\' && !inEscapeSeq) {
          inEscapeSeq = true;
          continue;
        }
        if (c == '"' && !inEscapeSeq) {
          *value = process_esc(*value);
          return TOKEN_TYPE_QUOTED_STRING;
        }
        continue;
      case TOKEN_STATE_TOKEN_TYPE_COMMENT:
        if (c == '\n' || c == '\r') {
          return TOKEN_TYPE_COMMENT;
        }
        *value += c;
        continue;
      case TOKEN_STATE_TOKEN_TYPE_NORMAL:
        if (c == ' ' || c == '\t' || c == '\n' || c == '\t' ||
            c == ';' || c == '{' || c == '}' || c == '\'' || c == '"') {
          input->unget();
          return TOKEN_TYPE_NORMAL;
        }
        *value += c;
        continue;
    }
  }

  // If we get here, we reached the end of the file.
  if (state == TOKEN_STATE_SINGLE_QUOTE ||
      state == TOKEN_STATE_DOUBLE_QUOTE) {
    return TOKEN_TYPE_ERROR;
  }

  return TOKEN_TYPE_EOF;
}

bool NginxConfigParser::Parse(std::istream* config_file, NginxConfig* config) {
  std::stack<NginxConfig*> config_stack;
  config_stack.push(config);
  TokenType last_token_type = TOKEN_TYPE_START;
  TokenType token_type;
  int error = 0;
  int bracket_balance = 0;
  while (true) {
    std::string token;
    bool seenWhitespace = false; // flag set to true whenever whitespace encountered
    token_type = ParseToken(config_file, &token, seenWhitespace);
    //printf ("%s: %s\n", TokenTypeAsString(token_type), token.c_str());
    if (token_type == TOKEN_TYPE_ERROR) {
      break;
    }

    if (token_type == TOKEN_TYPE_COMMENT) {
      // Skip comments.
      continue;
    }

    if (token_type == TOKEN_TYPE_START) {
      // Error.
      break;
    } else if (token_type == TOKEN_TYPE_NORMAL || token_type == TOKEN_TYPE_QUOTED_STRING) {
      if (last_token_type == TOKEN_TYPE_START ||
          last_token_type == TOKEN_TYPE_STATEMENT_END ||
          last_token_type == TOKEN_TYPE_START_BLOCK ||
          last_token_type == TOKEN_TYPE_END_BLOCK ||
          // enforce space in case of normal before quote and quote before quote
          seenWhitespace && (token_type == TOKEN_TYPE_NORMAL || token_type == TOKEN_TYPE_QUOTED_STRING) && (last_token_type == TOKEN_TYPE_NORMAL || last_token_type == TOKEN_TYPE_QUOTED_STRING)) {
        if (last_token_type != TOKEN_TYPE_NORMAL && last_token_type != TOKEN_TYPE_QUOTED_STRING) {
          config_stack.top()->statements_.emplace_back(
              new NginxConfigStatement);
        }
        config_stack.top()->statements_.back().get()->tokens_.push_back(
            token);
      } else {
        if (!seenWhitespace && (token_type == TOKEN_TYPE_NORMAL && last_token_type == TOKEN_TYPE_QUOTED_STRING || 
          token_type == TOKEN_TYPE_QUOTED_STRING && last_token_type == TOKEN_TYPE_NORMAL))
            error = 1;
        // Error.
        break;
      }
    } else if (token_type == TOKEN_TYPE_STATEMENT_END) {
      if (last_token_type != TOKEN_TYPE_NORMAL && last_token_type != TOKEN_TYPE_QUOTED_STRING) {
        // Error.
        break;
      }
    } else if (token_type == TOKEN_TYPE_START_BLOCK) {
      bracket_balance++;
      if (last_token_type != TOKEN_TYPE_NORMAL && last_token_type != TOKEN_TYPE_QUOTED_STRING) {
        // Error.
        break;
      }
      NginxConfig* const new_config = new NginxConfig;
      config_stack.top()->statements_.back().get()->child_block_.reset(
          new_config);
      config_stack.push(new_config);
    } else if (token_type == TOKEN_TYPE_END_BLOCK) {
      bracket_balance--;
      if (last_token_type != TOKEN_TYPE_STATEMENT_END && last_token_type != TOKEN_TYPE_END_BLOCK) {
        // Error.
        break;
      }
      config_stack.pop();
    } else if (token_type == TOKEN_TYPE_EOF) {
      if (last_token_type != TOKEN_TYPE_STATEMENT_END &&
          last_token_type != TOKEN_TYPE_END_BLOCK && last_token_type != TOKEN_TYPE_START) {
        // Error.
        break;
      }
      if (bracket_balance != 0)
        break;
      if (bracket_balance != 0)
        break;
      return true;
    } 
    else {
      // Error. Unknown token.
      break;
    }
    last_token_type = token_type;
  }
  if (error == 1) {
    printf ("No whitespace between %s and %s\n",
          TokenTypeAsString(last_token_type),
          TokenTypeAsString(token_type));
  }
  else {
    printf ("Bad transition from %s to %s\n",
            TokenTypeAsString(last_token_type),
            TokenTypeAsString(token_type));
  }
  return false;
}

bool NginxConfigParser::Parse(const char* file_name, NginxConfig* config) {
  std::ifstream config_file;
  config_file.open(file_name);
  if (!config_file.good()) {
    printf ("Failed to open config file: %s\n", file_name);
    return false;
  }

  const bool return_value =
      Parse(dynamic_cast<std::istream*>(&config_file), config);
  config_file.close();
  return return_value;
}

bool NginxConfigParser::GetPortNumber(const char* file_name, NginxConfig* config, short* port) {
  *config = NginxConfig();
  if (!Parse(file_name, config))
    return false;
  else if (config != nullptr) { // handle empty config
    /* need to look for this structure:
    server {
      ...
      listen <port #>
      ...
    }
    */
    for (auto statement : config->statements_) {
      std::vector<std::string> tokens = statement->tokens_;
      // find outer "server" declaration
      if (tokens.size() >= 1 && tokens[0] == "server") {
        NginxConfig* child_block = statement->child_block_.get();
        if (child_block != nullptr) {
          for (auto block_statement : child_block->statements_) {
            std::vector<std::string> block_tokens = block_statement->tokens_;
            // look for listen <port #> in child block
            if (block_tokens.size() >= 2 && block_tokens[0] == "listen" && is_number(block_tokens[1])) {
              *port = stoi(block_tokens[1]);
              return true;
            }
          }
        }
      }
    }
  }
  printf ("No port number found\n");
  return false;
}

// helper function for converting escape sequences into the chars they represent
// only support for '\\', '\'' inside "", and '"' inside ''
std::string process_esc(std::string token) {
  std::string processed_token = "";
  bool inEscapeSeq = false;
  for (auto c : token) {
    if (c == '\\' && !inEscapeSeq) {
        inEscapeSeq = true;
        continue;
    }
    if (inEscapeSeq)
        inEscapeSeq = false;
    processed_token.push_back(c);
  }
  return processed_token;
}

bool is_number(const std::string& s) {
    std::string::const_iterator it = s.begin();
    while (it != s.end() && std::isdigit(*it)) ++it;
    return !s.empty() && it == s.end();
}