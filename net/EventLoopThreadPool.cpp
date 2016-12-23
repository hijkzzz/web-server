#include <net/EventLoopThreadPool.h>

#include <net/EventLoop.h>
#include <net/EventLoopThread.h>

#include <assert.h>

EventLoopThreadPool::EventLoopThreadPool(EventLoop *baseLoop)
        : baseLoop_(baseLoop),
          started_(false),
          numThreads_(0),
          next_(0) {
}

EventLoopThreadPool::~EventLoopThreadPool() {
    // Don't delete loop, it's stack variable
}

void EventLoopThreadPool::start() {
    assert(!started_);
    baseLoop_->assertInLoopThread();

    started_ = true;

    for (int i = 0; i < numThreads_; ++i) {
        EventLoopThread *t = new EventLoopThread();
        std::unique_ptr<EventLoopThread> ut(t);
        threads_.push_back(std::move(ut));
        loops_.push_back(t->startLoop());
    }
}

EventLoop *EventLoopThreadPool::getNextLoop() {
    baseLoop_->assertInLoopThread();
    EventLoop *loop = baseLoop_;

    if (!loops_.empty()) {
        // round-robin
        loop = loops_[next_];
        ++next_;
        if (static_cast<size_t>(next_) >= loops_.size()) {
            next_ = 0;
        }
    }
    return loop;
}
