#include <net/Logging.h>

#include <sstream>

#include <string.h>

__thread char t_errnobuf[512];

const char *strerror_tl(int savedErrno) {
    // GNU-specific
    return strerror_r(savedErrno, t_errnobuf, sizeof t_errnobuf);
}

std::string threadString(const std::thread::id &threadId) {
    std::stringstream ss;
    ss << threadId;
    return ss.str();
}
