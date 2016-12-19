#ifndef NET_SOCKET_H
#define NET_SOCKET_H

#include <base/NonCopyable.h>

class InetAddress;

class Socket : NonCopyable{
public:
    explicit Socket(int sockfd)
            : sockfd_(sockfd) {}

    ~Socket();

    int fd() const { return sockfd_; }
    void bindAddress(const InetAddress &localaddr);
    void listen();
    int accept(InetAddress *peeraddr);
    void setReuseAddr(bool on);

private:
    const int sockfd_;
};

#endif
