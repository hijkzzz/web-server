#ifndef NET_EVENTLOOP_H
#define NET_EVENTLOOP_H

#include <boost/noncopyable.hpp>

class EventLoop : private boost::noncopyable {
public:
    EventLoop();
    ~EventLoop();

    void loop();

    void assertInLoopThread() {
        if (!isInLoopThread()) {
            abortNotInLoopThread();
        }
    }

    bool isInLoopThread() const { return threadId_ == CurrentThread::tid(); }

private:
    void abortNotInLoopThread();

    bool        looping_;
    const pid_t threadId_;
};

#endif //NET_EVENTLOOP_H
