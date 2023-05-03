#ifndef HTTPHEADER_H
#define HTTPHEADER_H

#include <string>

class HTTPHeader {
 public:
  std::string name;  // Case insensitive
  std::string value;
};

#endif