#include <net/EventLoopThread.h>

#include <net/EventLoop.h>

EventLoopThread::EventLoopThread()
        : loop_(NULL),
          exiting_(false),
          thread_(nullptr) {
}

EventLoopThread::~EventLoopThread() {
    exiting_ = true;
    loop_->quit();
    if (thread_.get() != nullptr)
        thread_->join();
}

EventLoop *EventLoopThread::startLoop() {
    {
        thread_.reset(new std::thread(&EventLoopThread::threadFunc, this));
        std::unique_lock<std::mutex> lock(mutex_);
        // 等待线程运行
        while (loop_ == nullptr) {
            cond_.wait(lock);
        }
    }

    return loop_;
}

void EventLoopThread::threadFunc() {
    EventLoop loop;

    {
        std::unique_lock<std::mutex> lock(mutex_);
        loop_ = &loop;
        cond_.notify_one();
    }

    loop.loop();
    loop_ = nullptr;
}
