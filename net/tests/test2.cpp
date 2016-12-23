/*
 * 定时器测试
 */
#include <net/EventLoop.h>

#include <unistd.h>
int cnt = 0;
EventLoop *g_loop;

void printTid() {
    printf("pid = %d \n", getpid());
    printf("now %ld\n", std::chrono::steady_clock::now().time_since_epoch().count() / 1000);
}

void print(const char *msg) {
    printf("msg %ld %s\n", std::chrono::steady_clock::now().time_since_epoch().count() / 1000, msg);
    if (++cnt == 20) {
        g_loop->quit();
    }
}

int main() {
    printTid();
    EventLoop loop;
    g_loop = &loop;

    print("main");
    loop.runAfter(100, std::bind(print, "once100"));
    loop.runAfter(200, std::bind(print, "once200"));
    loop.runAfter(300, std::bind(print, "once300"));
    loop.runEvery(200, std::bind(print, "every200"));
    loop.runEvery(300, std::bind(print, "every300"));

    loop.loop();
    print("main loop exits");
    sleep(1);
}