#ifndef NET_CALLBACKS_H
#define NET_CALLBACKS_H

#include <functional>
#include <memory>
#include <chrono>

class TcpConnection;
class Buffer;

using Clock = std::chrono::steady_clock;
using TcpConnectionPtr = std::shared_ptr<TcpConnection>;

using TimerCallback =  std::function<void()>;
using ConnectionCallback = std::function<void(const TcpConnectionPtr &)>;
using MessageCallback = std::function<void(const TcpConnectionPtr &, Buffer *data, Clock::time_point)>;
using CloseCallback = std::function<void(const TcpConnectionPtr &)>;

#endif
