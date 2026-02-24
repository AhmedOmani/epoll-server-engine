#include "EventHandler.hpp"
#include "EventLoop.hpp"
#include <iostream>
#include <unistd.h>

EventLoop::EventLoop() {
    epollFd = epoll_create1(0);
    if (epollFd == -1) {
        std::cerr << "Failed to create epoll instance!" << "\n";
        exit(1);
    }
}

EventLoop::~EventLoop() {
    close(epollFd);
}

bool EventLoop::addEvent(EventHandler* handler , uint32_t events) {
    struct epoll_event event;
    event.events = events;
    event.data.ptr = handler;
    return epoll_ctl(epollFd , EPOLL_CTL_ADD , handler->getFd() , &event) != -1;
}

int EventLoop::wait(std::vector<epoll_event>& activeEvents) {
    activeEvents.resize(MAX_EVENTS);
    return epoll_wait(epollFd , activeEvents.data() , MAX_EVENTS , 1000);
}