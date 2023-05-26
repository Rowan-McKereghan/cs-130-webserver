#ifndef FILESYSTEM_MUTEX_H
#define FILESYSTEM_MUTEX_H

#include <shared_mutex>

// not ideal to make the mutex a global variable, but the alternative is declaring it in server.h and passing it all the
// way down as a parameter, which is much worse
std::shared_mutex filesystem_mutex;

#endif
