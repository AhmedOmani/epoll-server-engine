#pragma once
#include "EventLoop.hpp"
#include "Connection.hpp"
#include <unordered_map>
#include <netinet/in.h>

class TcpServer {
private:
    int serverSocket;
    EventLoop eventLoop;
    std::unordered_map<int, Connection*> connections;

    bool setupSocket(int port);
    void handleNewConnection();
    void removeClient(int clientFd);

public:
    TcpServer(int port);
    ~TcpServer();
    void start();
};