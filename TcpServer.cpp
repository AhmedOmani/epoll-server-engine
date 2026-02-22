#include "TcpServer.hpp"
#include "Utils.hpp"
#include <iostream>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>

TcpServer::TcpServer(int port) {
    if (setupSocket(port)) {
        eventLoop.addEvent(serverSocket, EPOLLIN | EPOLLET);
    }
}

TcpServer::~TcpServer() {
    for (auto &pair: connections) {
        delete pair.second;
    }
    close(serverSocket);
}

bool TcpServer::setupSocket(int port) {
    serverSocket = socket(AF_INET, SOCK_STREAM , 0);
    int opt = 1;
    setsockopt(serverSocket , SOL_SOCKET , SO_REUSEADDR | SO_REUSEPORT , &opt , sizeof(opt));

    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    bind(serverSocket , (struct sockaddr*)& address , sizeof(address));
    listen(serverSocket , SOMAXCONN);
    return makeNonBlocking(serverSocket);
}

void TcpServer::handleNewConnection() {
    while(true) {
        struct sockaddr_in clientAddress;
        socklen_t clientAddressLength = sizeof(clientAddress);
        int clientSocket = accept(serverSocket , (struct sockaddr*)& clientAddress, &clientAddressLength);
        
        if (clientSocket == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) break;
            continue;
        }

        makeNonBlocking(clientSocket);
        if(!eventLoop.addEvent(clientSocket , EPOLLIN | EPOLLET)) {
            std::cout << "Failed to add client socket to epoll!" << std::endl;
            close(clientSocket);
            continue;
        }

        connections[clientSocket] = new Connection(clientSocket);
    }
}

void TcpServer::removeClient(int clientFd) {
    auto it = connections.find(clientFd);
    if (it != connections.end()) {
        delete it->second;
        connections.erase(it);
    }
}

void TcpServer::start() {
    std::cout << "Server started, entering EventLoop..." << std::endl;
    std::vector<epoll_event> activeEvents;

    while(true) {
        int numEvents = eventLoop.wait(activeEvents);

        for (int i = 0 ; i < numEvents ; i++) {
            int fd = activeEvents[i].data.fd;

            if (fd == serverSocket) {
                handleNewConnection();
            } 
            else {
                Connection* connection = connections[fd];
                if (!connection->handleRead()) {
                    removeClient(fd);
                }
            }
        }
    }
}