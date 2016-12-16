#ifndef NET_EVENTLOOP_H
#define NET_EVENTLOOP_H

#include <base/NonCopyable.h>

#include <thread>
#include <memory>
#include <vector>

class Poller;
class Channel;

class EventLoop : NonCopyable {
public:
    EventLoop();
    ~EventLoop();

    void loop();
    void quit();

    // internal use only
    void updateChannel(Channel *channel);

    void assertInLoopThread() {
        if (!isInLoopThread()) {
            abortNotInLoopThread();
        }
    }

    bool isInLoopThread() const { return threadId_ == std::this_thread::get_id(); }

private:
    void abortNotInLoopThread();

    typedef std::vector<Channel *> ChannelList;

    bool                    looping_;
    bool                    quit_;
    const std::thread::id   threadId_;
    std::unique_ptr<Poller> poller_;
    ChannelList             activeChannels_;
};

#endif //NET_EVENTLOOP_H
