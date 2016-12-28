#include <net/EventLoop.h>
#include <net/InetAddress.h>
#include <http/HttpServer.h>

int main() {
    int port = 8000;
    int threadNum = 4;
    std::string root = "./www";

    EventLoop loop;
    HttpServer server(&loop,
                      InetAddress(port),
                      "speedX",
                      root);
    server.setThreadNum(threadNum);
    server.start();

    loop.loop();
    return 0;
}