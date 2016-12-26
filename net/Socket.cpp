#include <net/Socket.h>

#include <net/SocketsOps.h>
#include <net/InetAddress.h>

#include <strings.h>

Socket::~Socket() {
    sockets::close(sockfd_);
}

void Socket::bindAddress(const InetAddress &addr) {
    sockets::bindOrDie(sockfd_, addr.getSockAddrInet());
}

void Socket::listen() {
    sockets::listenOrDie(sockfd_);
}

int Socket::accept(InetAddress *peeraddr) {
    struct sockaddr_in addr;
    bzero(&addr, sizeof addr);
    int connfd = sockets::accept(sockfd_, &addr);
    if (connfd >= 0) {
        peeraddr->setSockAddrInet(addr);
    }
    return connfd;
}

void Socket::setReuseAddr(bool on) {
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR,
                 &optval, sizeof optval);
}


void Socket::shutdownWrite() {
    sockets::shutdownWrite(sockfd_);
}


void Socket::setTcpNoDelay(bool on) {
  int optval = on ? 1 : 0;
  ::setsockopt(sockfd_, IPPROTO_TCP, TCP_NODELAY,
               &optval, static_cast<socklen_t>(sizeof optval));
}

void Socket::setKeepAlive(bool on) {
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_, SOL_SOCKET, SO_KEEPALIVE,
                 &optval, static_cast<socklen_t>(sizeof optval));
}
