#include "TcpServer.hpp"
#include "Utils.hpp"
#include <iostream>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
#include <chrono>
#include <cassert>
#include <utility>
#include <list>

using namespace std;

TcpServer::TcpServer(int port) {
    int serverSocket = setupSocket(port);
    this->acceptor = new Acceptor(this, serverSocket);
    eventLoop.addEvent(this->acceptor, EPOLLIN | EPOLLET);
}

TcpServer::~TcpServer() {
    for (auto &pair: connections) {
        delete pair.second.first;
    }
    close(acceptor->getFd());
}

int TcpServer::setupSocket(int port) {
    int serverSocket = socket(AF_INET, SOCK_STREAM , 0);
    int opt = 1;
    setsockopt(serverSocket , SOL_SOCKET , SO_REUSEADDR | SO_REUSEPORT , &opt , sizeof(opt));

    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    bind(serverSocket , (struct sockaddr*)& address , sizeof(address));
    listen(serverSocket , SOMAXCONN);
    if(!makeNonBlocking(serverSocket)) {
        cout << "Failed to make server socket non-blocking!" << endl;
        exit(1);
    }
    return serverSocket;
}


void TcpServer::removeClient(int clientFd) {
    auto it = connections.find(clientFd);
    if (it != connections.end()) {
        auto nodeList = it->second.second;
        timeoutList.erase(nodeList);
        delete it->second.first;
        connections.erase(it);
    }

}

void TcpServer::checkTimeout() {

    if (timeoutList.empty()) return;

    auto now = std::chrono::system_clock::now();
    
    while (true) {
        auto it = timeoutList.begin();

        if (it == timeoutList.end()) break;
        
        int fd = *it;
        auto connection = connections[fd].first;
        long long diff = chrono::duration_cast<chrono::seconds>(now - connection->getLastActiveTime()).count();
        
        if (diff < 3) 
            break;

        removeClient(fd);
    }

}

void TcpServer::updateLastActiveTime(int fd) {
    //1. One single hash map lookup.
    auto &record = connections[fd]; // pair<Connection*, list<int>::iterator>
    
    //2. Update the last active time
    record.first->setLastActiveTime(chrono::system_clock::now());

    //3. O(1) Snip out the list using saved iterator
    timeoutList.erase(record.second);

    //4. O(1) Push back to the end of the list cause it is most recent connection
    timeoutList.push_back(fd);

    //5. Update the map connection with the new iterator
    record.second = prev(timeoutList.end());
}

void TcpServer::start() {
    std::cout << "Server started, entering EventLoop..." << std::endl;
    std::vector<epoll_event> activeEvents;

    while(true) {
        
        checkTimeout();

        int numEvents = eventLoop.wait(activeEvents);

        for (int i = 0 ; i < numEvents ; i++) {
            EventHandler* handler = static_cast<EventHandler*>(activeEvents[i].data.ptr);
            handler->handleEvent();
        }
    }
}