#ifndef HTTPHEADER_H
#define HTTPHEADER_H

#include <string>

class HTTPHeader {
public:
	std::string name; // Case insensitive
	std::string value;
	  // Default constructor
  HTTPHeader() {}

  // Constructor that takes a name and value
  HTTPHeader(const std::string& name, const std::string& value)
      : name(name), value(value) {}
};

#endif