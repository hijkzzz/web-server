#ifndef NET_TIMER_H
#define NET_TIMER_H

#include <net/Callbacks.h>
#include <chrono>

#include <boost/noncopyable.hpp>

class Timer : boost::noncopyable {
public:
    using Clock = std::chrono::steady_clock;

    Timer(const TimerCallback &cb, Clock::time_point when, int interval)
            : callback_(cb),
              expiration_(when),
              interval_(interval),
              repeat_(interval > 0) {}

    void run() const {
        callback_();
    }

    Clock::time_point expiration() const { return expiration_; }
    bool repeat() const { return repeat_; }
    void restart(Clock::time_point now);

private:
    const TimerCallback callback_;
    Clock::time_point   expiration_;
    const int           interval_;
    const bool          repeat_;
};

#endif
