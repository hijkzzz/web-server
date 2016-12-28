#ifndef NET_EVENTLOOP_H
#define NET_EVENTLOOP_H

#include <net/Callbacks.h>

#include <boost/noncopyable.hpp>

#include <thread>
#include <memory>
#include <vector>
#include <functional>
#include <mutex>

class EPoller;
class Channel;

class EventLoop : boost::noncopyable {
public:
    using Functor = std::function<void()>;

    EventLoop();
    ~EventLoop();

    void loop();
    void quit();

    Clock::time_point pollReturnTime() const { return pollReturnTime_; }

    // thread safe
    void runInLoop(const Functor&& cb);
    void queueInLoop(const Functor&& cb);

    void wakeup(); // 唤醒循环
    void updateChannel(Channel *channel);
    void removeChannel(Channel* channel);

    void assertInLoopThread() {
        if (!isInLoopThread()) {
            abortNotInLoopThread();
        }
    }

    bool isInLoopThread() const { return threadId_ == std::this_thread::get_id(); }

private:
    void abortNotInLoopThread();
    void handleRead();
    void doPendingFunctors();

    using ChannelList =  std::vector<Channel *>;

    bool                        looping_;
    bool                        quit_;
    bool                        callingPendingFunctors_;
    const std::thread::id       threadId_;
    Clock::time_point           pollReturnTime_;
    std::unique_ptr<EPoller>    poller_;
    int                         wakeupFd_;
    std::unique_ptr<Channel>    wakeupChannel_;

    ChannelList                 activeChannels_;
    std::mutex                  mutex_;
    std::vector<Functor>        pendingFunctors_;
};

#endif //NET_EVENTLOOP_H
