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
// IO Multiplexing with poll(2).
class Poller : NonCopyable {
public:
    typedef std::vector<Channel *> ChannelList;
    typedef std::chrono::system_clock::time_point Timestamp;

    Poller(EventLoop *loop);
    ~Poller();

    // 只能在 Loop 线程调用
    // Polls the I/O events.
    Timestamp poll(int timeoutMs, ChannelList *activeChannels);

    // 只能在 Loop 线程调用
    // Changes the interested I/O events.
    void updateChannel(Channel *channel);

    void assertInLoopThread() { ownerLoop_->assertInLoopThread(); }

private:
    void fillActiveChannels(int numEvents,
                            ChannelList *activeChannels) const;

    typedef std::vector<struct pollfd> PollFdList;
    typedef std::map<int, Channel *>   ChannelMap;

    EventLoop  *ownerLoop_;
    PollFdList pollfds_;
    ChannelMap channels_;
};

#endif
