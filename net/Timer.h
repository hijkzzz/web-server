#ifndef NET_TIMER_H
#define NET_TIMER_H

#include <net/Callbacks.h>

#include <boost/noncopyable.hpp>

#include <atomic>

class Timer : boost::noncopyable {
public:
    Timer(const TimerCallback &cb, Clock::time_point when, int interval)
            : callback_(cb),
              expiration_(when),
              interval_(interval),
              repeat_(interval > 0),
              sequence_(++s_numCreated_){}


    void run() const {
        callback_();
    }

    Clock::time_point expiration() const { return expiration_; }
    bool repeat() const { return repeat_; }
    int64_t sequence() const { return sequence_; }

    void restart(Clock::time_point now);

private:
    const TimerCallback callback_;
    Clock::time_point   expiration_;
    const int           interval_;
    const bool          repeat_;
    const int64_t       sequence_;

    static std::atomic<std::int64_t> s_numCreated_;
};

#endif
