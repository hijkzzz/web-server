#include <net/TimerQueue.h>

#include <net/Logging.h>
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

struct timespec howMuchTimeFromNow(Clock::time_point when) {
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

void readTimerfd(int timerfd, Clock::time_point now) {
    uint64_t howmany;
    ssize_t  n = ::read(timerfd, &howmany, sizeof howmany);
    LOG_TRACE << "TimerQueue::handleRead() " << howmany << " at " << now.time_since_epoch().count();
    if (n != sizeof howmany) {
        LOG_ERROR << "TimerQueue::handleRead() reads " << n << " bytes instead of 8";
    }
}

void resetTimerfd(int timerfd, Clock::time_point expiration) {
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
          timers_(),
          callingExpiredTimers_(false){
    timerfdChannel_.setReadCallback(
            std::bind(&TimerQueue::handleRead, this));
    // we are always reading the timerfd, we disarm it with timerfd_settime.
    timerfdChannel_.enableReading();
}

TimerQueue::~TimerQueue() {
    ::close(timerfd_);
}

/// 线程安全的版本
TimerId TimerQueue::addTimer(const TimerCallback &cb,
                             Clock::time_point when,
                             int interval) {
    std::shared_ptr<Timer> timer(new Timer(cb, when, interval));
    // 转发到 IO 线程
    loop_->runInLoop(std::bind(&TimerQueue::addTimerInLoop, this, timer));
    return TimerId(timer, timer->sequence());
}

void TimerQueue::addTimerInLoop(std::shared_ptr<Timer> &timer) {
    loop_->assertInLoopThread();
    bool earliestChanged = insert(timer);

    if (earliestChanged) {
        resetTimerfd(timerfd_, timer->expiration());
    }
}

/// 线程安全的版本
void TimerQueue::cancel(TimerId timerId) {
    loop_->runInLoop(
            std::bind(&TimerQueue::cancelInLoop, this, timerId));
}

void TimerQueue::cancelInLoop(TimerId timerId) {
    loop_->assertInLoopThread();
    assert(timers_.size() == activeTimers_.size());
    std::shared_ptr<Timer> t = timerId.timer_.lock();
    if (!t) {
        LOG_TRACE << "Timer has been deleted";
        return;
    }

    ActiveTimer              timer(t, timerId.sequence_);
    ActiveTimerSet::iterator it = activeTimers_.find(timer);
    if (it != activeTimers_.end()) {
        size_t n = timers_.erase(Entry(it->first->expiration(), it->first));
        assert(n == 1);
        (void) n;
        activeTimers_.erase(it);
    // 对应"自注销"的情况
    // Timer 在 expired 中，不能调用 restart
    } else if (callingExpiredTimers_) {
        cancelingTimers_.insert(timer);
    }
    assert(timers_.size() == activeTimers_.size());
}

void TimerQueue::handleRead() {
    loop_->assertInLoopThread();
    Clock::time_point now = Clock::now();
    readTimerfd(timerfd_, now);

    std::vector<Entry> expired = getExpired(now);

    callingExpiredTimers_ = true;
    cancelingTimers_.clear();
    // safe to callback outside critical section
    for (std::vector<Entry>::iterator it = expired.begin();
         it != expired.end(); ++it) {
        it->second->run();
    }
    callingExpiredTimers_ = false;

    // restart Timer and reset timerfd
    reset(expired, now);
    // expired
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

    for (Entry entry : expired) {
        ActiveTimer timer(entry.second, entry.second->sequence());
        size_t n = activeTimers_.erase(timer);
        assert(n == 1); (void)n;
    }

    assert(timers_.size() == activeTimers_.size());
    return expired;
}

void TimerQueue::reset(std::vector<Entry> &expired, Clock::time_point now) {
    // max 作为无效值
    Clock::time_point nextExpire = nextExpire.max();

    for (std::vector<Entry>::iterator it = expired.begin();
         it != expired.end();++it) {

        ActiveTimer timer(it->second, it->second->sequence());
        if (it->second->repeat()  && cancelingTimers_.find(timer) == cancelingTimers_.end()) {
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

bool TimerQueue::insert(std::shared_ptr<Timer> &timer) {
    bool                earliestChanged = false;
    Clock::time_point   when            = timer->expiration();
    TimerList::iterator it              = timers_.begin();
    if (it == timers_.end() || when < it->first) {
        earliestChanged = true;
    }

    {
        std::pair<TimerList::iterator, bool> result = timers_.insert(
                make_pair(when, timer));
        assert(result.second);
        (void) result;
    }

    {
        std::pair<ActiveTimerSet::iterator, bool> result = activeTimers_.insert(
                make_pair(timer, timer->sequence()));
        assert(result.second);
        (void) result;
    }

    assert(timers_.size() == activeTimers_.size());
    return earliestChanged;
}