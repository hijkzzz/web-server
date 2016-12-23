#ifndef NET_TCPCONNECTION_H
#define NET_TCPCONNECTION_H

#include <net/Callbacks.h>
#include <net/InetAddress.h>
#include <net/Buffer.h>

#include <boost/noncopyable.hpp>

#include <memory>

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

    void setConnectionCallback(const ConnectionCallback &cb) { connectionCallback_ = cb; }
    void setMessageCallback(const MessageCallback &cb) { messageCallback_ = cb; }
    // Internal use only.
    void setCloseCallback(const CloseCallback& cb) { closeCallback_ = cb; }

    // called when TcpServer accepts a new connection
    void connectEstablished();   // should be called only once
    // called when TcpServer has removed me from its map
    void connectDestroyed();  // should be called only once

private:
    enum StateE {
        kConnecting, kConnected, kDisconnected
    };

    void setState(StateE s) { state_ = s; }
    void handleRead(Clock::time_point receiveTime);
    void handleWrite();
    void handleClose();
    void handleError();

    EventLoop                *loop_;
    std::string              name_;
    StateE                   state_;  // FIXME: use atomic variable
    // we don't expose those classes to client.
    std::unique_ptr<Socket>  socket_;
    std::unique_ptr<Channel> channel_;
    InetAddress              localAddr_;
    InetAddress              peerAddr_;
    ConnectionCallback       connectionCallback_;
    MessageCallback          messageCallback_;
    CloseCallback            closeCallback_;
    Buffer                   inputBuffer_;
};

#endif
