#include <net/EventLoop.h>
#include <net/InetAddress.h>
#include <http/HttpServer.h>

int main() {
    int port = 8000;
    int threadNum = 4;

    // parse command

    EventLoop loop;
    HttpServer server(&loop, InetAddress(port));
    server.setThreadNum(threadNum);
    server.start();
    return 0;
}