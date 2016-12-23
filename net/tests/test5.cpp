/*
 * 测试 TcpServer 和 TcpConnection
 */
#include <net/EventLoop.h>
#include <net/TcpServer.h>
#include <net/InetAddress.h>
#include <net/TcpConnection.h>

#include <stdio.h>
#include <unistd.h>

void onConnection(const TcpConnectionPtr &conn) {
    if (conn->connected()) {
        printf("onConnection(): new connection [%s] from %s\n",
               conn->name().c_str(),
               conn->peerAddress().toHostPort().c_str());
    } else {
        printf("onConnection(): connection [%s] is down\n",
               conn->name().c_str());
    }
}

void onMessage(const TcpConnectionPtr &conn,
               Buffer *data,
               Clock::time_point receiveTime) {
    printf("onMessage(): received %zd bytes from connection [%s]\n",
           data->readableBytes(), conn->name().c_str());
}

int main() {
    printf("main(): pid = %d\n", getpid());

    InetAddress listenAddr(9981);
    EventLoop   loop;

    TcpServer server(&loop, listenAddr);
    server.setConnectionCallback(onConnection);
    server.setMessageCallback(onMessage);
    server.start();

    loop.loop();
}
