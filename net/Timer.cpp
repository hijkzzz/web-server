#include <net/Timer.h>

std::atomic<std::int64_t> Timer::s_numCreated_;

void Timer::restart(Clock::time_point now) {
    if (repeat_) {
        expiration_ = now +
                std::chrono::microseconds(interval_);
    } else {
        expiration_ = expiration_.max();
    }
}
