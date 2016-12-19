/*
 *  测试 EventLoopThread 以及 runInLoop
 */


#include <net/EventLoop.h>
#include <net/EventLoopThread.h>

#include <iostream>

#include <unistd.h>

void runInThread() {
    std::cout << "runInThread: pid = " << getpid() << ", tid = " << std::this_thread::get_id() << std::endl;
}

int main() {
    std::cout << "main(): pid = " << getpid() << ", tid = " << std::this_thread::get_id() << std::endl;

    EventLoopThread loopThread;
    EventLoop *loop = loopThread.startLoop();
    loop->runInLoop(runInThread);
    sleep(1);
    loop->runAfter(500, runInThread);
    sleep(3);

    loop->quit();

    std::cout << "main(): exit" << std::endl;
    return 0;
}

