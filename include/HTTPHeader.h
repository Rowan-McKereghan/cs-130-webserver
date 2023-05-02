#ifndef HTTPHEADER_H
#define HTTPHEADER_H

#include <string>

class header {
public:
	std::string name; // Case insensitive
	std::string value;
};

#endif