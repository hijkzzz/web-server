#ifndef NET_TIMERID_H
#define NET_TIMERID_H

#include <memory>

class Timer;

///
/// An opaque identifier, for canceling Timer.
///
class TimerId {
public:
    TimerId(std::shared_ptr<Timer> timer = nullptr, int64_t seq = 0)
            : timer_(timer),
              sequence_(seq) {
    }

    // default copy-ctor, dtor and assignment are okay

    friend class TimerQueue;

private:
    std::weak_ptr<Timer> timer_;
    int64_t sequence_;
};


#endif
