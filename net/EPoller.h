#ifndef NET_POLLER_H
#define NET_POLLER_H

#include <net/EventLoop.h>
#include <net/Callbacks.h>

#include <boost/noncopyable.hpp>

#include <vector>
#include <map>
#include <chrono>

struct epoll_event;
class Channel;

// EPoller 并不拥有 Channel
class EPoller : boost::noncopyable {
public:
    using ChannelList =  std::vector<Channel *>;

    EPoller(EventLoop *loop);
    ~EPoller();

    // 只能在 Loop 线程调用
    Clock::time_point poll(int timeoutMs, ChannelList *activeChannels);

    // 只能在 Loop 线程调用
    void updateChannel(Channel *channel);
    void removeChannel(Channel* channel);

    void assertInLoopThread() { ownerLoop_->assertInLoopThread(); }

private:
    static const int kInitEventListSize = 16;

    void fillActiveChannels(int numEvents,
                            ChannelList *activeChannels) const;
    void update(int operation, Channel* channel);

    using EventList =  std::vector<struct epoll_event>;
    using ChannelMap =  std::map<int, Channel *>;

    EventLoop  *ownerLoop_;
    int        epollfd_;
    EventList  events_;
    ChannelMap channels_;
};

#endif
