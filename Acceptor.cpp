#include "Acceptor.hpp"
#include "Utils.hpp"
#include "Connection.hpp"
#include "TcpServer.hpp"
#include <sys/socket.h>
#include <errno.h>
#include <iostream>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/epoll.h>

using namespace std;

Acceptor::Acceptor(TcpServer* server ,int serverSocket) : serverSocket(serverSocket) , server(server) {}

int Acceptor::getFd() const {
    return serverSocket;
}

void Acceptor::handleEvent() {
    handleNewConnection();
}

void Acceptor::handleNewConnection() {
    while(true) {
        struct sockaddr_in clientAddress;
        socklen_t clientAddressLength = sizeof(clientAddress);
        int clientSocket = accept(serverSocket , (struct sockaddr*)& clientAddress , &clientAddressLength);

        if (clientSocket == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) break;
            continue;
        }

        makeNonBlocking(clientSocket);
        
        //Assigning callbacks
        auto onCloseCallback = [this](int fd) { this->server->removeClient(fd); };
        auto onReadCallback = [this](int fd) { this->server->updateLastActiveTime(fd); };

        //Create connection object
        auto connection = new Connection(clientSocket , std::chrono::system_clock::now() , onCloseCallback , onReadCallback);

        //set the last active time and add to timeout list
        server->timeoutList.push_back(clientSocket);
        auto listIterator = prev(server->timeoutList.end());
        server->connections[clientSocket] = {connection , listIterator};

        if (!server->eventLoop.addEvent(connection, EPOLLIN | EPOLLET)) {
            std::cout << "Failed to add client socket to epoll!" << std::endl;
            close(clientSocket);
            continue;
        }
    }
}