#ifndef NET_TCPCONNECTION_H
#define NET_TCPCONNECTION_H

#include <net/Callbacks.h>
#include <net/InetAddress.h>
#include <net/Buffer.h>

#include <boost/noncopyable.hpp>
#include <boost/any.hpp>

#include <memory>
#include <atomic>

class Channel;
class EventLoop;
class Socket;

class TcpConnection : boost::noncopyable,
                      public std::enable_shared_from_this<TcpConnection> {
public:

    // User should not create this object.
    TcpConnection(EventLoop *loop,
                  const std::string &name,
                  int sockfd,
                  const InetAddress &localAddr,
                  const InetAddress &peerAddr);

    ~TcpConnection();

    EventLoop *getLoop() const { return loop_; }
    const std::string &name() const { return name_; }
    const InetAddress &localAddress() { return localAddr_; }
    const InetAddress &peerAddress() { return peerAddr_; }
    bool connected() const { return state_ == kConnected; }

    // Thread safe.
    void send(const std::string &message);
    void send(Buffer *buf);
    void shutdown();

    void setContext(const boost::any &context) { context_ = context; }
    const boost::any &getContext() const { return context_; }
    boost::any *getMutableContext() { return &context_; }

    void setConnectionCallback(const ConnectionCallback &cb) { connectionCallback_ = cb; }
    void setMessageCallback(const MessageCallback &cb) { messageCallback_ = cb; }
    // Internal use only.
    void setCloseCallback(const CloseCallback& cb) { closeCallback_ = cb; }

    void connectEstablished();   // should be called only once
    // called when TcpServer has removed me from its map
    void connectDestroyed();  // should be called only once

private:
    enum StateE { kConnecting, kConnected, kDisconnecting, kDisconnected, };

    void setState(StateE s) { state_ = s; }
    void handleRead(Clock::time_point receiveTime);
    void handleWrite();
    void handleClose();
    void handleError();

    void sendInLoop(const std::string& message);
    void shutdownInLoop();

    EventLoop                *loop_;
    std::string              name_;
    std::atomic<StateE>      state_;
    // we don't expose those classes to client.
    std::unique_ptr<Socket>  socket_;
    std::unique_ptr<Channel> channel_;
    InetAddress              localAddr_;
    InetAddress              peerAddr_;
    ConnectionCallback       connectionCallback_;
    MessageCallback          messageCallback_;
    CloseCallback            closeCallback_;
    Buffer                   inputBuffer_;
    Buffer                   outputBuffer_;
    boost::any               context_;
};

#endif
