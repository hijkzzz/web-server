#ifndef NET_CALLBACKS_H
#define NET_CALLBACKS_H

#include <functional>
#include <memory>

class TcpConnection;

using TcpConnectionPtr = std::shared_ptr<TcpConnection>;

using TimerCallback =  std::function<void()>;
using ConnectionCallback = std::function<void(const TcpConnectionPtr &)>;
using MessageCallback = std::function<void(const TcpConnectionPtr &, const char *data, ssize_t len)>;
using CloseCallback = std::function<void(const TcpConnectionPtr &)>;

#endif
