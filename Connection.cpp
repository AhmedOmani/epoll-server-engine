#include "Connection.hpp"
#include <unistd.h>
#include <errno.h>
#include <iostream>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <chrono>

Connection::Connection(int fd , std::chrono::system_clock::time_point lastActive, std::function<void(int)> onCloseCallback, std::function<void(int)> onReadCallback) : fd(fd) , lastActive(lastActive) , onCloseCallback(onCloseCallback), onReadCallback(onReadCallback) {}

Connection::~Connection() {
    close(fd);
}

int Connection::getFd() const {
    return fd;
}

void Connection::handleRead() {
    char buffer[4096] = {0};

    while (true) {
        int bytesRead = read(fd , buffer , sizeof(buffer) - 1);

        if (bytesRead > 0) {
            buffer[bytesRead] = '\0';
            readBuffer += buffer;
        }
        else if (bytesRead == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                if (readBuffer.find("\r\n\r\n") != std::string::npos) {
                    onReadCallback(this->fd);
                    processRequest();
                    readBuffer.clear();
                }
                return ;
            }
            std::cerr << "Read error: " << strerror(errno) << "\n";
            onCloseCallback(this->fd);
            return;
        }
        else if(bytesRead == 0) {
            onCloseCallback(this->fd);
            return;
        }
    }
}

void Connection::handleEvent() {
    handleRead();
}

void Connection::processRequest() {
    std::string response = "Hello World from Omani Server!\n"; 
    sendResponse(response.length() , response); 
}

void Connection::sendResponse(int contentLength , const std::string &response) {
    std::string fullResponse = "HTTP/1.1 200 OK\r\n"
                               "Connection: keep-alive\r\n"
                               "Content-Length: " + std::to_string(contentLength) + "\r\n\r\n" + response;
    
    send(fd , fullResponse.c_str() , fullResponse.size() , MSG_NOSIGNAL);
}

std::chrono::system_clock::time_point Connection::getLastActiveTime() {
    return lastActive;
}

void Connection::setLastActiveTime(std::chrono::system_clock::time_point lastActive) {
    this->lastActive = lastActive;
}


