#include <net/TcpServer.h>

#include <net/Logging.h>
#include <net/EventLoop.h>
#include <net/Acceptor.h>
#include <net/InetAddress.h>
#include <net/SocketsOps.h>
#include <net/TcpConnection.h>

#include <functional>

TcpServer::TcpServer(EventLoop *loop, const InetAddress &listenAddr)
        : loop_(loop),
          name_(listenAddr.toHostPort()),
          acceptor_(new Acceptor(loop, listenAddr)),
          started_(false),
          nextConnId_(1) {

    if (loop == nullptr) {
        LOG_FATAL << "loop_ == nullptr";
    }

    acceptor_->setNewConnectionCallback(
            std::bind(&TcpServer::newConnection, this, std::placeholders::_1, std::placeholders::_2));
}

TcpServer::~TcpServer() {
}

void TcpServer::start() {
    if (!started_) {
        started_ = true;
    }

    if (!acceptor_->listenning()) {
        loop_->runInLoop(
                std::bind(&Acceptor::listen, acceptor_.get()));
    }
}

void TcpServer::newConnection(int sockfd, const InetAddress &peerAddr) {
    loop_->assertInLoopThread();
    char buf[32];
    snprintf(buf, sizeof buf, "#%d", nextConnId_);
    ++nextConnId_;
    std::string      connName = name_ + buf;

    LOG_INFO << "TcpServer::newConnection [" << name_
             << "] - new connection [" << connName
             << "] from " << peerAddr.toHostPort();
    InetAddress      localAddr(sockets::getLocalAddr(sockfd));
    // FIXME poll with zero timeout to double confirm the new connection
    TcpConnectionPtr conn(
            new TcpConnection(loop_, connName, sockfd, localAddr, peerAddr));
    connections_[connName] = conn;
    conn->setConnectionCallback(connectionCallback_);
    conn->setMessageCallback(messageCallback_);
    conn->setCloseCallback(
            std::bind(&TcpServer::removeConnection, this, std::placeholders::_1));
    conn->connectEstablished();
}

void TcpServer::removeConnection(const TcpConnectionPtr &conn) {
    loop_->assertInLoopThread();
    LOG_INFO << "TcpServer::removeConnection [" << name_
             << "] - connection " << conn->name();
    size_t n = connections_.erase(conn->name());
    assert(n == 1);
    (void) n;
    loop_->queueInLoop(
            std::bind(&TcpConnection::connectDestroyed, conn));
}
