#include <net/TimerQueue.h>

#include <base/Logging.h>
#include <net/EventLoop.h>
#include <net/Timer.h>
#include <net/TimerId.h>

#include <sys/timerfd.h>


int createTimerfd() {
    int timerfd = ::timerfd_create(CLOCK_MONOTONIC,
                                   TFD_NONBLOCK | TFD_CLOEXEC);
    if (timerfd < 0) {
        LOG_FATAL << "Failed in timerfd_create";
    }
    return timerfd;
}

struct timespec howMuchTimeFromNow(TimerQueue::Clock::time_point when) {
    int64_t microseconds =
                    std::chrono::duration_cast<std::chrono::microseconds>(
                            when - std::chrono::steady_clock::now()).count();
    if (microseconds < 100) {
        microseconds = 100;
    }
    struct timespec ts;
    ts.tv_sec  = static_cast<time_t>(microseconds / 1000);
    ts.tv_nsec = static_cast<long>(
            (microseconds % 1000) * 1000);
    return ts;
}

void readTimerfd(int timerfd, TimerQueue::Clock::time_point now) {
    uint64_t howmany;
    ssize_t  n = ::read(timerfd, &howmany, sizeof howmany);
    LOG_TRACE << "TimerQueue::handleRead() " << howmany << " at " << now.time_since_epoch().count();
    if (n != sizeof howmany) {
        LOG_ERROR << "TimerQueue::handleRead() reads " << n << " bytes instead of 8";
    }
}

void resetTimerfd(int timerfd, TimerQueue::Clock::time_point expiration) {
    // wake up loop by timerfd_settime()
    struct itimerspec newValue;
    struct itimerspec oldValue;
    bzero(&newValue, sizeof newValue);
    bzero(&oldValue, sizeof oldValue);
    newValue.it_value = howMuchTimeFromNow(expiration);
    int ret = ::timerfd_settime(timerfd, 0, &newValue, &oldValue);
    if (ret) {
        LOG_ERROR << "timerfd_settime()";
    }
}

TimerQueue::TimerQueue(EventLoop *loop)
        : loop_(loop),
          timerfd_(createTimerfd()),
          timerfdChannel_(loop, timerfd_),
          timers_() {
    timerfdChannel_.setReadCallback(
            std::bind(&TimerQueue::handleRead, this));
    // we are always reading the timerfd, we disarm it with timerfd_settime.
    timerfdChannel_.enableReading();
}

TimerQueue::~TimerQueue() {
    ::close(timerfd_);
}

TimerId TimerQueue::addTimer(const TimerCallback &cb,
                             Clock::time_point when,
                             int interval) {
    std::shared_ptr<Timer> timer(new Timer(cb, when, interval));
    loop_->assertInLoopThread();
    bool earliestChanged = insert(timer);

    if (earliestChanged) {
        resetTimerfd(timerfd_, timer->expiration());
        LOG_TRACE << "reset Timerfd when" << when.time_since_epoch().count() / 1000;
    }
    return TimerId(timer.get());
}

void TimerQueue::handleRead() {
    loop_->assertInLoopThread();
    Clock::time_point now = Clock::now();
    readTimerfd(timerfd_, now);

    std::vector<Entry> expired = getExpired(now);

    // safe to callback outside critical section
    for (std::vector<Entry>::iterator it = expired.begin();
         it != expired.end(); ++it) {
        it->second->run();
    }

    // restart Timer and reset timerfd
    reset(expired, now);
    // ~vector<Entry> will clean Timers
}

std::vector<TimerQueue::Entry> TimerQueue::getExpired(Clock::time_point now) {
    std::vector<Entry>  expired;
    Entry               sentry(now, std::shared_ptr<Timer>(nullptr));
    TimerList::iterator it     = timers_.lower_bound(sentry);
    assert(it == timers_.end() || now < it->first);

    for (std::set<Entry >::iterator i = timers_.begin(); i != it; ++i) {
        // http://stackoverflow.com/questions/25515284/is-it-possible-to-move-an-item-out-of-a-stdset
        expired.push_back(*i);
    }
    timers_.erase(timers_.begin(), it);

    return expired;
}

void TimerQueue::reset(std::vector<Entry> &expired, Clock::time_point now) {
    // max 作为无效值
    Clock::time_point nextExpire = nextExpire.max();

    for (std::vector<Entry>::iterator it = expired.begin();
         it != expired.end();++it) {
        if (it->second->repeat()) {
            it->second->restart(now);
            insert(it->second);
        }
    }

    if (!timers_.empty()) {
        nextExpire = timers_.begin()->second->expiration();
    }

    if (nextExpire != nextExpire.max()) {
        LOG_TRACE << "reset Timerfd nextExpire " << nextExpire.time_since_epoch().count() / 1000;
        resetTimerfd(timerfd_, nextExpire);
    }
}

bool TimerQueue::insert(std::shared_ptr<Timer>& timer) {
    bool earliestChanged = false;
    Clock::time_point when = timer->expiration();
    TimerList::iterator it  = timers_.begin();
    if (it == timers_.end() || when < it->first) {
        earliestChanged = true;
    }

    std::pair<TimerList::iterator, bool> result = timers_.insert(
            make_pair(when, timer));
    assert(result.second);
    return earliestChanged;
}