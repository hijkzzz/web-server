#include <net/Logging.h>
#include <string.h>

__thread char t_errnobuf[512];

const char *strerror_tl(int savedErrno) {
    // GNU-specific
    return strerror_r(savedErrno, t_errnobuf, sizeof t_errnobuf);
}