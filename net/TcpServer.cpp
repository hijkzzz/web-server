#include <net/TcpServer.h>

#include <net/Logging.h>
#include <net/EventLoop.h>
#include <net/Acceptor.h>
#include <net/InetAddress.h>
#include <net/SocketsOps.h>
#include <net/TcpConnection.h>
#include <net/EventLoopThreadPool.h>

#include <functional>

TcpServer::TcpServer(EventLoop *loop, const InetAddress &listenAddr)
        : loop_(loop),
          name_(listenAddr.toHostPort()),
          acceptor_(new Acceptor(loop, listenAddr)),
          threadPool_(new EventLoopThreadPool(loop)),
          started_(false),
          nextConnId_(1) {

    if (loop == nullptr) {
        log_err("loop_ == nullptr");
        abort();
    }

    acceptor_->setNewConnectionCallback(
            std::bind(&TcpServer::newConnection, this, std::placeholders::_1, std::placeholders::_2));
}

TcpServer::~TcpServer() {
}

void TcpServer::setThreadNum(int numThreads) {
    assert(0 <= numThreads);
    threadPool_->setThreadNum(numThreads);
}

void TcpServer::start() {
    if (!started_) {
        started_ = true;
        threadPool_->start();
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

    log_info("TcpServer::newConnection [%s] - new connection [%s] from %s",
             name_.c_str(),
             connName.c_str(),
             peerAddr.toHostPort().c_str());
    InetAddress      localAddr(sockets::getLocalAddr(sockfd));
    // FIXME poll with zero timeout to double confirm the new connection
    EventLoop* ioLoop = threadPool_->getNextLoop();
    TcpConnectionPtr conn(
            new TcpConnection(ioLoop, connName, sockfd, localAddr, peerAddr));
    connections_[connName] = conn;
    conn->setConnectionCallback(connectionCallback_);
    conn->setMessageCallback(messageCallback_);
    conn->setCloseCallback(
            std::bind(&TcpServer::removeConnection, this, std::placeholders::_1)); // FIXME: unsafe
    ioLoop->runInLoop(std::bind(&TcpConnection::connectEstablished, conn));
}

void TcpServer::removeConnection(const TcpConnectionPtr& conn)
{
    // FIXME: unsafe
    loop_->runInLoop(std::bind(&TcpServer::removeConnectionInLoop, this, conn));
}

void TcpServer::removeConnectionInLoop(const TcpConnectionPtr& conn)
{
    loop_->assertInLoopThread();
    log_info("TcpServer::removeConnectionInLoop [%s] - connection %s",
             name_.c_str(),
             conn->name().c_str());
    size_t n = connections_.erase(conn->name());
    assert(n == 1);
    (void)n;
    EventLoop* ioLoop = conn->getLoop();
    ioLoop->queueInLoop(
            std::bind(&TcpConnection::connectDestroyed, conn));
}
