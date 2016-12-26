#ifndef BASE_LOGGING_H
#define BASE_LOGGING_H

#include <string>
#include <thread>

#include <stdio.h>
#include <errno.h>
#include <string.h>

#define __STDC_FORMAT_MACROS

#ifdef NDEBUG
#define debug(M, ...)
#else
#define debug(M, ...) fprintf(stderr, "[DEBUG] %s:%d: " M "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#endif

#define log_err(M, ...) fprintf(stderr, "[ERROR] " M "\n",  ##__VA_ARGS__)
#define log_warn(M, ...) fprintf(stderr, "[WARN] " M "\n", ##__VA_ARGS__)
#define log_info(M, ...) fprintf(stderr, "[INFO] " M "\n", ##__VA_ARGS__)

const char *strerror_tl(int savedErrno);
std::string threadString(const std::thread::id &threadId);

#endif
