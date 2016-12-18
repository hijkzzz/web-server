#ifndef NET_EVENTLOOP_H
#define NET_EVENTLOOP_H

#include <base/NonCopyable.h>
#include <net/Callbacks.h>
#include <net/TimerId.h>

#include <thread>
#include <memory>
#include <vector>

class Poller;
class Channel;
class TimerQueue;
class TimerId;

class EventLoop : NonCopyable {
public:
    using Clock = std::chrono::steady_clock;

    EventLoop();
    ~EventLoop();

    void loop();
    void quit();

    Clock::time_point pollReturnTime() const { return pollReturnTime_; }

    TimerId runAt(const Clock::time_point& time, const TimerCallback& cb);
    TimerId runAfter(int delay, const TimerCallback& cb);
    TimerId runEvery(int interval, const TimerCallback& cb);

    void updateChannel(Channel *channel);

    void assertInLoopThread() {
        if (!isInLoopThread()) {
            abortNotInLoopThread();
        }
    }

    bool isInLoopThread() const { return threadId_ == std::this_thread::get_id(); }

private:
    void abortNotInLoopThread();

    using ChannelList =  std::vector<Channel *>;

    bool                    looping_;
    bool                    quit_;
    const std::thread::id   threadId_;
    Clock::time_point pollReturnTime_;
    std::unique_ptr<Poller> poller_;
    std::unique_ptr<TimerQueue> timerQueue_;
    ChannelList             activeChannels_;
};

#endif //NET_EVENTLOOP_H
