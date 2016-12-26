#include <net/EPoller.h>

#include <net/Logging.h>
#include <net/Channel.h>

#include <assert.h>
#include <strings.h>
#include <poll.h>
#include <sys/epoll.h>

// On Linux, the constants of poll(2) and epoll(4)
// are expected to be the same.
static_assert(EPOLLIN == POLLIN);
static_assert(EPOLLPRI == POLLPRI);
static_assert(EPOLLOUT == POLLOUT);
static_assert(EPOLLRDHUP == POLLRDHUP);
static_assert(EPOLLERR == POLLERR);
static_assert(EPOLLHUP == POLLHUP);

namespace {
    const int kNew     = -1;
    const int kAdded   = 1;
    const int kDeleted = 2;
}

EPoller::EPoller(EventLoop *loop)
        : ownerLoop_(loop),
          epollfd_(::epoll_create1(EPOLL_CLOEXEC)),
          events_(kInitEventListSize) {
    if (epollfd_ < 0) {
        log_err("EPoller::EPoller");
    }
}

EPoller::~EPoller() {
    ::close(epollfd_);
}

Clock::time_point EPoller::poll(int timeoutMs, ChannelList *activeChannels) {
    int       numEvents = ::epoll_wait(epollfd_,
                                       events_.data(),
                                       static_cast<int>(events_.size()),
                                       timeoutMs);
    int savedErrno = errno;
    Clock::time_point now = Clock::now();
    if (numEvents > 0) {
        log_info("%d events happended", numEvents);
        fillActiveChannels(numEvents, activeChannels);
        if (static_cast<size_t>(numEvents) == events_.size()) {
            events_.resize(events_.size() * 2);
        }
    } else if (numEvents == 0) {
        log_info("nothing happended");
    } else {
        if (savedErrno != EINTR) {
            errno = savedErrno;
            log_err("EPollPoller::poll()");
        }
    }
    return now;
}

void EPoller::fillActiveChannels(int numEvents,
                                 ChannelList *activeChannels) const {
    assert(static_cast<size_t>(numEvents) <= events_.size());
    for (int i = 0; i < numEvents; ++i) {
        Channel *channel = static_cast<Channel *>(events_[i].data.ptr);
#ifndef NDEBUG
        int                        fd = channel->fd();
        ChannelMap::const_iterator it = channels_.find(fd);
        assert(it != channels_.end());
        assert(it->second == channel);
#endif
        channel->set_revents(events_[i].events);
        activeChannels->push_back(channel);
    }
}

void EPoller::updateChannel(Channel *channel) {
    assertInLoopThread();
    log_info("update fd = %d events = %d",channel->fd(), channel->events());
    const int index = channel->index();
    if (index == kNew || index == kDeleted) {
        // a new one, add with EPOLL_CTL_ADD
        int fd = channel->fd();
        if (index == kNew) {
            assert(channels_.find(fd) == channels_.end());
            channels_[fd] = channel;
        } else {   // index == kDeleted
            assert(channels_.find(fd) != channels_.end());
            assert(channels_[fd] == channel);
        }
        channel->set_index(kAdded);
        update(EPOLL_CTL_ADD, channel);
    } else {
        // update existing one with EPOLL_CTL_MOD/DEL
        int fd = channel->fd();
        (void) fd;
        assert(channels_.find(fd) != channels_.end());
        assert(channels_[fd] == channel);
        assert(index == kAdded);
        if (channel->isNoneEvent()) {
            update(EPOLL_CTL_DEL, channel);
            channel->set_index(kDeleted);
        } else {
            update(EPOLL_CTL_MOD, channel);
        }
    }
}

void EPoller::removeChannel(Channel *channel) {
    assertInLoopThread();
    int fd = channel->fd();
    log_info("remove fd = %d", fd);
    assert(channels_.find(fd) != channels_.end());
    assert(channels_[fd] == channel);
    assert(channel->isNoneEvent());
    int index = channel->index();
    assert(index == kAdded || index == kDeleted);
    size_t n = channels_.erase(fd);
    (void) n;
    assert(n == 1);

    if (index == kAdded) {
        update(EPOLL_CTL_DEL, channel);
    }
    channel->set_index(kNew);
}

void EPoller::update(int operation, Channel *channel) {
    struct epoll_event event;
    bzero(&event, sizeof event);
    event.events   = channel->events();
    event.data.ptr = channel;
    int fd = channel->fd();
    if (::epoll_ctl(epollfd_, operation, fd, &event) < 0) {
        if (operation == EPOLL_CTL_DEL) {
            log_err("epoll_ctl op = %d fd = %d", operation, fd);
        } else {
            log_err("epoll_ctl op = %d fd = %d", operation, fd);
        }
    }
}