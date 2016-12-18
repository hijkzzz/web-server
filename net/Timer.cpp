#include <net/Timer.h>

void Timer::restart(Clock::time_point now) {
    if (repeat_) {
        expiration_ = now +
                std::chrono::microseconds(interval_);
    } else {
        expiration_ = expiration_.max();
    }
}
