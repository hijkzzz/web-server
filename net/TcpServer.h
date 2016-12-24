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
class EventLoopThreadPool;

class TcpServer : boost::noncopyable {
public:
    TcpServer(EventLoop *loop, const InetAddress &listenAddr);
    ~TcpServer();  // force out-line dtor, for scoped_ptr members.

    /// @param numThreads
    /// - 0 means all I/O in loop's thread, no thread will created.
    ///   this is the default value.
    /// - 1 means all I/O in another thread.
    /// - N means a thread pool with N threads, new connections
    ///   are assigned on a round-robin basis.
    void setThreadNum(int numThreads);

    void start(); // thread safe
    void setConnectionCallback(const ConnectionCallback &cb) { connectionCallback_ = cb; }
    void setMessageCallback(const MessageCallback &cb) { messageCallback_ = cb; }

    const std::string& ipPort() const { return name_; }
    EventLoop* getLoop() const { return loop_; }

private:
    // not thread safe, but in loop
    void newConnection(int sockfd, const InetAddress &peerAddr);
    // thread safe
    void removeConnection(const TcpConnectionPtr &conn);
    // not thread safe, but in loop
    void removeConnectionInLoop(const TcpConnectionPtr &conn);

    using ConnectionMap =  std::map <std::string, TcpConnectionPtr>;

    EventLoop                            *loop_;  // the acceptor loop
    const std::string                    name_;
    std::unique_ptr<Acceptor>            acceptor_; // avoid revealing Acceptor
    std::unique_ptr<EventLoopThreadPool> threadPool_;
    ConnectionCallback                   connectionCallback_;
    MessageCallback                      messageCallback_;
    bool                                 started_;
    int                                  nextConnId_;  // always in loop thread
    ConnectionMap                        connections_;
};

#endif
