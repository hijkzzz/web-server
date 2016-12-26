#include <net/EventLoop.h>
#include <net/InetAddress.h>
#include <http/HttpServer.h>

int main() {
    int port = 8000;
    int threadNum = 2;

    EventLoop loop;
    HttpServer server(&loop, InetAddress(port), "speedX");
    server.setThreadNum(threadNum);
    server.start();

    loop.loop();
    return 0;
}