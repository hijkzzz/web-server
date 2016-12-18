#include <net/EventLoop.h>

#include <unistd.h>
// 定时器测试
int cnt = 0;
EventLoop *g_loop;

void printTid() {
    printf("pid = %d", getpid());
    printf("now %ld\n", std::chrono::steady_clock::now().time_since_epoch().count());
}

void print(const char *msg) {
    printf("msg %ld %s\n", std::chrono::steady_clock::now().time_since_epoch().count(), msg);
    if (++cnt == 20) {
        g_loop->quit();
    }
}

int main() {
    printTid();
    EventLoop loop;
    g_loop = &loop;

    print("main");
    loop.runAfter(1, std::bind(print, "once1"));
    loop.runAfter(2, std::bind(print, "once2"));
    loop.runAfter(3, std::bind(print, "once3"));
    loop.runEvery(2, std::bind(print, "every2"));
    loop.runEvery(3, std::bind(print, "every3"));

    loop.loop();
    print("main loop exits");
    sleep(1);
}