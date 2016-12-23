#ifndef NET_EVENTLOOPTHREADPOOL_H
#define NET_EVENTLOOPTHREADPOOL_H

#include <boost/noncopyable.hpp>

#include <vector>
#include <memory>

class EventLoop;
class EventLoopThread;

class EventLoopThreadPool : boost::noncopyable {
public:
    EventLoopThreadPool(EventLoop *baseLoop);
    ~EventLoopThreadPool();

    void setThreadNum(int numThreads) { numThreads_ = numThreads; }
    void start();

    EventLoop *getNextLoop();

private:
    EventLoop                                     *baseLoop_;
    bool                                          started_;
    int                                           numThreads_;
    int                                           next_;  // always in loop thread
    std::vector<std::unique_ptr<EventLoopThread>> threads_;
    std::vector<EventLoop *>                      loops_;
};


#endif //NET_EVENTLOOPTHREADPOOL_H
