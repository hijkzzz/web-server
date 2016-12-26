#include <net/TcpConnection.h>

#include <net/Logging.h>
#include <net/Channel.h>
#include <net/EventLoop.h>
#include <net/Socket.h>
#include <net/SocketsOps.h>
#include <net/Buffer.h>

#include <functional>

TcpConnection::TcpConnection(EventLoop *loop,
                             const std::string &nameArg,
                             int sockfd,
                             const InetAddress &localAddr,
                             const InetAddress &peerAddr)
        : loop_(loop),
          name_(nameArg),
          state_(kConnecting),
          socket_(new Socket(sockfd)),
          channel_(new Channel(loop, sockfd)),
          localAddr_(localAddr),
          peerAddr_(peerAddr) {
    if (loop_ == nullptr) {
        log_err("init TcpConnection: loop is nullptr");
        abort();
    }

    debug("TcpConnection::ctor[%s] at %p fd = %d", name_.c_str(), this, sockfd);
    channel_->setReadCallback(
            std::bind(&TcpConnection::handleRead, this, std::placeholders::_1));
    channel_->setWriteCallback(
            std::bind(&TcpConnection::handleWrite, this));
    channel_->setCloseCallback(
            std::bind(&TcpConnection::handleClose, this));
    channel_->setErrorCallback(
            std::bind(&TcpConnection::handleError, this));
}

TcpConnection::~TcpConnection() {
    debug("TcpConnection::ctor[%s] at %p fd = %d", name_.c_str(), this, sockfd);
    assert(state_ == kDisconnected);
}

void TcpConnection::send(Buffer *buf) {
    if (state_ == kConnected) {
        if (loop_->isInLoopThread()) {
            sendInLoop(buf->peek(), buf->readableBytes());
        } else {
            loop_->runInLoop(
                    std::bind(&TcpConnection::sendInLoop,
                                this,
                                buf->peek(), buf->readableBytes()));
        }
        buf->retrieveAll();
    }
}

void TcpConnection::send(std::string message) {
    if (state_ == kConnected) {
        if (loop_->isInLoopThread()) {
            sendInLoop(message.data(), message.size());
        } else {
            loop_->runInLoop(
                    std::bind(&TcpConnection::sendInLoop,
                                this,
                                message.data(), message.size()));
        }
    }
}

void TcpConnection::send(const char *message, int len) {
        if (state_ == kConnected) {
        if (loop_->isInLoopThread()) {
            sendInLoop(message, len);
        } else {
            loop_->runInLoop(
                    std::bind(&TcpConnection::sendInLoop, this, message, len));
        }
    }
}

void TcpConnection::sendInLoop(const char *message, int len) {
    loop_->assertInLoopThread();
    ssize_t nwrote = 0;
    // if no thing in output queue, try writing directly
    if (!channel_->isWriting() && outputBuffer_.readableBytes() == 0) {
        nwrote = ::write(channel_->fd(), message, len);
        if (nwrote >= 0) {
            if (static_cast<int>(nwrote) < len) {
                log_info("I am going to write more data");
            }
            // 此处应有"写完成回调"
        } else {
            nwrote = 0;
            if (errno != EWOULDBLOCK) {
                log_info("TcpConnection::sendInLoop");
            }
        }
    }

    assert(nwrote >= 0);
    if (static_cast<int>(nwrote) < len) {
        outputBuffer_.append(message + nwrote, len - nwrote);
        if (!channel_->isWriting()) {
            // 等待可写
            channel_->enableWriting();
        }
    }
}

void TcpConnection::shutdown() {
    // FIXME: use compare and swap
    if (state_ == kConnected) {
        setState(kDisconnecting);
        // FIXME: shared_from_this()?
        loop_->runInLoop(std::bind(&TcpConnection::shutdownInLoop, this));
    }
}

void TcpConnection::shutdownInLoop() {
    loop_->assertInLoopThread();
    if (!channel_->isWriting()) {
        // we are not writing
        socket_->shutdownWrite();
    }
}

void TcpConnection::forceClose() {
    // FIXME: use compare and swap
    if (state_ == kConnected || state_ == kDisconnecting) {
        setState(kDisconnecting);
        loop_->queueInLoop(std::bind(&TcpConnection::forceCloseInLoop, shared_from_this()));
    }
}

void TcpConnection::forceCloseInLoop() {
    loop_->assertInLoopThread();
    if (state_ == kConnected || state_ == kDisconnecting) {
        // as if we received 0 byte in handleRead();
        handleClose();
    }
}

void TcpConnection::setTcpNoDelay(bool on) {
    socket_->setTcpNoDelay(on);
}

void TcpConnection::connectEstablished() {
    loop_->assertInLoopThread();
    assert(state_ == kConnecting);
    setState(kConnected);
    channel_->enableReading();

    connectionCallback_(shared_from_this());
}

void TcpConnection::connectDestroyed() {
    loop_->assertInLoopThread();
    if (state_ == kConnected) {
        setState(kDisconnected);
        channel_->disableAll();

        connectionCallback_(shared_from_this());
    }
    channel_->remove();
}

void TcpConnection::handleRead(Clock::time_point receiveTime) {
    int savedErrno = 0;
    ssize_t n = inputBuffer_.readFd(channel_->fd(), &savedErrno);
    if (n > 0) {
        messageCallback_(shared_from_this(), &inputBuffer_, receiveTime);
    } else if (n == 0) {
        handleClose();
    } else {
        errno = savedErrno;
        log_err("TcpConnection::handleRead");
        handleError();
    }
}

void TcpConnection::handleWrite() {
    loop_->assertInLoopThread();
    if (channel_->isWriting()) {
        ssize_t n = ::write(channel_->fd(),
                            outputBuffer_.peek(),
                            outputBuffer_.readableBytes());
        if (n > 0) {
            outputBuffer_.retrieve(n);
            if (outputBuffer_.readableBytes() == 0) {
                channel_->disableWriting();
                // 此处应有"写完成回调"
                if (state_ == kDisconnecting) {
                    shutdownInLoop();
                }
            } else {
                log_info("I am going to write more data");
            }
        } else {
            log_info("TcpConnection::handleWrite");
        }
    } else {
        log_info("Connection is down, no more writing");
    }
}

void TcpConnection::handleClose() {
    loop_->assertInLoopThread();
    log_info("TcpConnection::handleClose state = %d", state_);
    assert(state_ == kConnected || state_ == kDisconnecting);
    // we don't close fd, leave it to dtor, so we can find leaks easily.
    channel_->disableAll();
    // must be the last line
    closeCallback_(shared_from_this());
}

void TcpConnection::handleError() {
    int err = sockets::getSocketError(channel_->fd());
    log_err("TcpConnection::handleError [%s] - SO_ERROR = %d %s", name_.c_str(), err, strerror_tl(err));
}
