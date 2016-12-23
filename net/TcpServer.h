#ifndef NET_TCPSERVER_H
#define NET_TCPSERVER_H

#include <net/Callbacks.h>

#include <boost/noncopyable.hpp>

#include <memory>
#include <map>
#include <string>

class Acceptor;
class EventLoop;
class InetAddress;

class TcpServer : boost::noncopyable {
public:
    TcpServer(EventLoop *loop, const InetAddress &listenAddr);
    ~TcpServer();  // force out-line dtor, for scoped_ptr members.

    void start(); // thread safe
    void setConnectionCallback(const ConnectionCallback &cb) { connectionCallback_ = cb; }
    void setMessageCallback(const MessageCallback &cb) { messageCallback_ = cb; }

private:
    // not thread safe, but in loop
    void newConnection(int sockfd, const InetAddress &peerAddr);
    void removeConnection(const TcpConnectionPtr& conn);

    using ConnectionMap =  std::map <std::string, TcpConnectionPtr>;

    EventLoop *loop_;  // the acceptor loop
    const std::string            name_;
    std::unique_ptr<Acceptor>    acceptor_; // avoid revealing Acceptor
    ConnectionCallback           connectionCallback_;
    MessageCallback              messageCallback_;
    bool                         started_;
    int                          nextConnId_;  // always in loop thread
    ConnectionMap                connections_;
};

#endif
