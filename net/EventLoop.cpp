#include <net/EventLoop.h>

#include <base/Logging.h>
#include <net/Poller.h>
#include <net/Channel.h>
#include <net/TimerQueue.h>

__thread EventLoop *t_loopInThisThread = 0;
// poll 超时时间
const int kPollTimeMs = 10000;

EventLoop::EventLoop()
        : looping_(false),
          quit_(false),
          threadId_(std::this_thread::get_id()),
          poller_(new Poller(this)),
          timerQueue_(new TimerQueue(this)) {
    LOG_TRACE << "EventLoop created " << this << " in thread " << threadId_;
    if (t_loopInThisThread) {
        LOG_FATAL << "Another EventLoop " << t_loopInThisThread
                  << " exists in this thread " << threadId_;
    } else {
        t_loopInThisThread = this;
    }
}

EventLoop::~EventLoop() {
    assert(!looping_);
    t_loopInThisThread = NULL;
}

void EventLoop::loop() {
    assert(!looping_);
    assertInLoopThread();
    looping_ = true;
    quit_    = false;

    while (!quit_) {
        activeChannels_.clear();
        pollReturnTime_ = poller_->poll(kPollTimeMs, &activeChannels_);
        for (ChannelList::iterator it = activeChannels_.begin();
             it != activeChannels_.end(); ++it) {
            (*it)->handleEvent();
        }
    }

    LOG_TRACE << "EventLoop " << this << " stop looping";
    looping_ = false;
}

void EventLoop::quit() {
    quit_ = true;
    // wakeup();
}

TimerId EventLoop::runAt(const Clock::time_point& time, const TimerCallback& cb) {
    return timerQueue_->addTimer(cb, time, 0.0);
}

TimerId EventLoop::runAfter(int delay, const TimerCallback& cb) {
    Clock::time_point time = Clock::now() + std::chrono::microseconds(delay);
    return runAt(time, cb);
}

TimerId EventLoop::runEvery(int interval, const TimerCallback& cb) {
    Clock::time_point time = Clock::now() + std::chrono::microseconds(interval);
    return timerQueue_->addTimer(cb, time, interval);
}

void EventLoop::updateChannel(Channel *channel) {
    assert(channel->ownerLoop() == this);
    assertInLoopThread();
    poller_->updateChannel(channel);
}

void EventLoop::abortNotInLoopThread() {
    LOG_FATAL << "EventLoop::abortNotInLoopThread - EventLoop " << this
              << " was created in threadId_ = " << threadId_
              << ", current thread id = " << std::this_thread::get_id();
}
