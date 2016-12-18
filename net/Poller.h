#ifndef NET_POLLER_H
#define NET_POLLER_H

#include <base/NonCopyable.h>
#include <net/EventLoop.h>

#include <vector>
#include <map>
#include <chrono>

struct pollfd;
class Channel;

// Poller 并不拥有 Channel
class Poller : NonCopyable {
public:
    using ChannelList =  std::vector<Channel *>;
    using Clock = std::chrono::steady_clock;

    Poller(EventLoop *loop);
    ~Poller();

    // 只能在 Loop 线程调用
    Clock::time_point poll(int timeoutMs, ChannelList *activeChannels);

    // 只能在 Loop 线程调用
    void updateChannel(Channel *channel);

    void assertInLoopThread() { ownerLoop_->assertInLoopThread(); }

private:
    void fillActiveChannels(int numEvents,
                            ChannelList *activeChannels) const;

    using PollFdList =  std::vector<struct pollfd>;
    using ChannelMap =  std::map<int, Channel *>;

    EventLoop  *ownerLoop_;
    PollFdList pollfds_;
    ChannelMap channels_;
};

#endif
