#ifndef NET_HTTPSERVER_H
#define NET_HTTPSERVER_H

#include <net/TcpServer.h>
#include <net/Callbacks.h>

#include <boost/noncopyable.hpp>

#include <functional>
#include <string>

class HttpRequest;
class HttpResponse;

class HttpServer : boost::noncopyable {
public:
    using HttpHandler =  std::function<void(const HttpRequest &, HttpResponse *)>;

    HttpServer(EventLoop *loop,
               const InetAddress &listenAddr);
    ~HttpServer();

    EventLoop *getLoop() const { return server_.getLoop(); }

    /// Not thread safe, callback be registered before calling start().
    void setHttpCallback(const HttpHandler &cb) {
        httpHandler_ = cb;
    }
    void setThreadNum(int numThreads) {
        server_.setThreadNum(numThreads);
    }

    void start();

private:
    void onConnection(const TcpConnectionPtr &conn);
    void onMessage(const TcpConnectionPtr &conn,
                   Buffer *buf,
                    Clock::time_point receiveTime);
    void onRequest(const TcpConnectionPtr &, const HttpRequest &);

    TcpServer    server_;
    HttpHandler httpHandler_;
    std::string  name_;
};

#endif
