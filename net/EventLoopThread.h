#ifndef NET_EVENTLOOPTHREAD_H
#define NET_EVENTLOOPTHREAD_H

#include <base/NonCopyable.h>

#include <thread>
#include <mutex>
#include <condition_variable>
#include <memory>

class EventLoop;

class EventLoopThread : NonCopyable{
public:
    EventLoopThread();
    ~EventLoopThread();

    EventLoop *startLoop();

private:
    void threadFunc();

    EventLoop                    *loop_;
    bool                         exiting_;
    std::unique_ptr<std::thread> thread_;
    std::mutex                   mutex_;
    std::condition_variable      cond_;
};

#endif
