#include <net/EventLoop.h>

#include <net/Logging.h>
#include <net/EPoller.h>
#include <net/Channel.h>

#include <signal.h>
#include <assert.h>
#include <sys/eventfd.h>


__thread EventLoop *t_loopInThisThread = 0;
// poll 超时时间
const int kPollTimeMs = 10000;

static int createEventfd() {
    int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (evtfd < 0) {
        log_err("Failed in eventfd");
        abort();
    }
    return evtfd;
}

class IgnoreSigPipe {
public:
    IgnoreSigPipe() {
        ::signal(SIGPIPE, SIG_IGN);
    }
};

// 构造函数中忽略 SIGPIPE
IgnoreSigPipe initObj;

EventLoop::EventLoop()
        : looping_(false),
          quit_(false),
          callingPendingFunctors_(false),
          threadId_(std::this_thread::get_id()),
          poller_(new EPoller(this)),
          wakeupFd_(createEventfd()),
          wakeupChannel_(new Channel(this, wakeupFd_)) {
    log_info("EventLoop created %p in thread %s", this, threadString(threadId_).c_str());
    if (t_loopInThisThread) {
        log_err("Another EventLoop %p exists in this thread %s", t_loopInThisThread,
                  threadString(threadId_).c_str());
    } else {
        t_loopInThisThread = this;
    }

    wakeupChannel_->setReadCallback(std::bind(&EventLoop::handleRead, this));
    // we are always reading the wakeupfd
    wakeupChannel_->enableReading();
}

EventLoop::~EventLoop() {
    assert(!looping_);
    wakeupChannel_->disableAll();
    wakeupChannel_->remove();
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
            (*it)->handleEvent(pollReturnTime_);
        }

        doPendingFunctors();
    }

    log_info("EventLoop %p stop looping", this);
    looping_ = false;
}

void EventLoop::quit() {
    quit_ = true;
    if (!isInLoopThread()) {
        wakeup();
    }
}

void EventLoop::runInLoop(const Functor &&cb) {
    if (isInLoopThread()) {
        cb();
    } else {
        queueInLoop(std::move(cb));
    }
}

void EventLoop::queueInLoop(const Functor &&cb) {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        pendingFunctors_.push_back(std::move(cb));
    }

    // 唤醒 EventLoop 处理 pendingFunctors
    if (!isInLoopThread() || callingPendingFunctors_) {
        wakeup();
    }
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
    log_err("EventLoop::abortNotInLoopThread - EventLoop %p was created in threadId_ = %s, current thread id = %s",
            this,
            threadString(threadId_).c_str(),
            threadString(std::this_thread::get_id()).c_str());
}

void EventLoop::wakeup() {
    uint64_t one = 1;
    ssize_t  n   = ::write(wakeupFd_, &one, sizeof one);
    if (n != sizeof one) {
        log_err("EventLoop::wakeup() writes %ld bytes instead of 8", n);
    }
}

void EventLoop::handleRead() {
    uint64_t one = 1;
    ssize_t  n   = ::read(wakeupFd_, &one, sizeof one);
    if (n != sizeof one) {
        log_err("EventLoop::handleRead() reads %ld bytes instead of 8", n);
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
