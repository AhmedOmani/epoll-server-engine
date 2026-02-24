#pragma once
#include "EventHandler.hpp"

class TcpServer; // for compiler to know the class is exists

class Acceptor : public EventHandler {
private:
    int serverSocket;
    TcpServer* server;
    void handleNewConnection();

public:
    Acceptor() = default;
    Acceptor(TcpServer* server , int serverSocket);
    int getFd() const override ;
    void handleEvent() override;
};