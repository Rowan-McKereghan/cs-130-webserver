#ifndef PRIVILEGED_DIRS
#define PRIVILEGED_DIRS

#include <string>
#include <vector>

const std::vector<std::string> privileged_dirs = {
    "/etc", "/usr/bin"};  // TODO: allow this to be configurable

#endif