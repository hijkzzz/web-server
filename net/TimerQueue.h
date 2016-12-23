#ifndef NET_TIMERQUEUE_H
#define NET_TIMERQUEUE_H

#include <net/Callbacks.h>
#include <net/Channel.h>

#include <boost/noncopyable.hpp>

#include <vector>
#include <set>
#include <utility>
#include <memory>

class EventLoop;
class Timer;
class TimerId;

class TimerQueue : boost::noncopyable {
public:
    TimerQueue(EventLoop *loop);
    ~TimerQueue();

    TimerId addTimer(const TimerCallback &cb,
                     Clock::time_point when,
                     int interval);

    // void cancel(TimerId timerId);

private:
    using Entry =  std::pair<Clock::time_point, std::shared_ptr<Timer>>;
    using TimerList =  std::set<Entry>;

    void addTimerInLoop(std::shared_ptr<Timer> timer);
    void handleRead(); // called when timerfd alarms
    std::vector<Entry> getExpired(Clock::time_point now); // move out all expired timers
    void reset(std::vector<Entry> &expired, Clock::time_point now);

    bool insert(std::shared_ptr<Timer>& timer);

    EventLoop *loop_;
    const int timerfd_;
    Channel   timerfdChannel_;
    TimerList timers_; // Timer list sorted by expiration
};


#endif
