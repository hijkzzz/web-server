#ifndef NET_TIMERID_H
#define NET_TIMERID_H

class Timer;

class TimerId {
public:
    explicit TimerId(Timer *timer)
            : value_(timer) {
    }

    // default copy-ctor, dtor and assignment are okay

private:
    Timer *value_;
};


#endif
