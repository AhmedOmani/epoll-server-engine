#pragma once
#include "EventHandler.hpp"
#include <sys/epoll.h>
#include <vector>

#define MAX_EVENTS 1024

class EventLoop {
private:
    int epollFd;

public:
    EventLoop();
    ~EventLoop();

    bool addEvent(EventHandler* handler , uint32_t events);
    int wait(std::vector<epoll_event>& activeEvents);
};