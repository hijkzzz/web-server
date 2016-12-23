#include <net/EventLoop.h>

#include <net/Logging.h>
#include <net/Poller.h>
#include <net/Channel.h>
#include <net/TimerQueue.h>

#include <sys/eventfd.h>

__thread EventLoop *t_loopInThisThread = 0;
// poll 超时时间
const int kPollTimeMs = 10000;

static int createEventfd() {
    int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (evtfd < 0) {
        LOG_ERROR << "Failed in eventfd";
        abort();
    }
    return evtfd;
}

EventLoop::EventLoop()
        : looping_(false),
          quit_(false),
          callingPendingFunctors_(false),
          threadId_(std::this_thread::get_id()),
          poller_(new Poller(this)),
          timerQueue_(new TimerQueue(this)),
          wakeupFd_(createEventfd()),
          wakeupChannel_(new Channel(this, wakeupFd_)) {
    LOG_TRACE << "EventLoop created " << this << " in thread " << threadId_;
    if (t_loopInThisThread) {
        LOG_FATAL << "Another EventLoop " << t_loopInThisThread
                  << " exists in this thread " << threadId_;
    } else {
        t_loopInThisThread = this;
    }

    wakeupChannel_->setReadCallback(std::bind(&EventLoop::handleRead, this));
    // we are always reading the wakeupfd
    wakeupChannel_->enableReading();
}

EventLoop::~EventLoop() {
    assert(!looping_);
    ::close(wakeupFd_);
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

        doPendingFunctors();
    }

    LOG_TRACE << "EventLoop " << this << " stop looping";
    looping_ = false;
}

void EventLoop::quit() {
    quit_ = true;
    if (!isInLoopThread()) {
        wakeup();
    }
}

void EventLoop::runInLoop(const Functor &cb) {
    if (isInLoopThread()) {
        cb();
    } else {
        queueInLoop(cb);
    }
}

void EventLoop::queueInLoop(const Functor &cb) {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        pendingFunctors_.push_back(cb);
    }

    // 唤醒 EventLoop 处理 pendingFunctors
    if (!isInLoopThread() || callingPendingFunctors_) {
        wakeup();
    }
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

void EventLoop::removeChannel(Channel *channel) {
    assert(channel->ownerLoop() == this);
    assertInLoopThread();
    poller_->removeChannel(channel);
}

void EventLoop::abortNotInLoopThread() {
    LOG_FATAL << "EventLoop::abortNotInLoopThread - EventLoop " << this
              << " was created in threadId_ = " << threadId_
              << ", current thread id = " << std::this_thread::get_id();
}

void EventLoop::wakeup() {
    uint64_t one = 1;
    ssize_t  n   = ::write(wakeupFd_, &one, sizeof one);
    if (n != sizeof one) {
        LOG_ERROR << "EventLoop::wakeup() writes " << n << " bytes instead of 8";
    }
}

void EventLoop::handleRead() {
    uint64_t one = 1;
    ssize_t  n   = ::read(wakeupFd_, &one, sizeof one);
    if (n != sizeof one) {
        LOG_ERROR << "EventLoop::handleRead() reads " << n << " bytes instead of 8";
    }
}

void EventLoop::doPendingFunctors() {
    std::vector<Functor> functors;
    callingPendingFunctors_ = true;

    {
        std::lock_guard<std::mutex> lock(mutex_);
        functors.swap(pendingFunctors_);
    }

    for (size_t i = 0; i < functors.size(); ++i) {
        functors[i]();
    }
    callingPendingFunctors_ = false;
}
