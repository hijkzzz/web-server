/*
 * 多线程测试
 */
#include <net/TcpServer.h>
#include <net/EventLoop.h>
#include <net/InetAddress.h>
#include <net/TcpConnection.h>

#include <sstream>
#include <string>

#include <stdio.h>
#include <unistd.h>


std::string getTid() {
    std::stringstream s;
    s << std::this_thread::get_id();
    return s.str();
}

void onConnection(const ::TcpConnectionPtr &conn) {
    if (conn->connected()) {
        printf("onConnection(): tid=%s new connection [%s] from %s\n",
               getTid().c_str(),
               conn->name().c_str(),
               conn->peerAddress().toHostPort().c_str());
    } else {
        printf("onConnection(): tid=%s connection [%s] is down\n",
               getTid().c_str(),
               conn->name().c_str());
    }
}

void onMessage(const ::TcpConnectionPtr &conn,
               ::Buffer *buf,
               Clock::time_point receiveTime) {
    printf("onMessage(): tid=%s received %zd bytes from connection [%s] at %ld\n",
           getTid().c_str(),
           buf->readableBytes(),
           conn->name().c_str(),
           receiveTime.time_since_epoch().count());

    conn->send(buf->retrieveAsString());
}

int main() {
    printf("main(): pid = %d\n", getpid());

    InetAddress listenAddr(9981);
    EventLoop   loop;

    TcpServer server(&loop, listenAddr);
    server.setConnectionCallback(onConnection);
    server.setMessageCallback(onMessage);
    server.setThreadNum(4);
    server.start();

    loop.loop();
}
