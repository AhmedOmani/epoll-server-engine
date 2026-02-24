#pragma once
#include "Acceptor.hpp"
#include "Connection.hpp"
#include "EventLoop.hpp"
#include <unordered_map>
#include <netinet/in.h>
#include <list>
#include <utility>


class TcpServer {
private:
    friend class Acceptor;
    Acceptor *acceptor;
    EventLoop eventLoop;
    std::unordered_map<int, std::pair<Connection*, std::list<int>::iterator>> connections;
    std::list<int>timeoutList;

    int setupSocket(int port);
    void removeClient(int clientFd);
    void updateLastActiveTime(int fd);
    void checkTimeout();

public:
    TcpServer(int port);
    ~TcpServer();
    void start();
};