#include "Connection.hpp"
#include <unistd.h>
#include <errno.h>
#include <iostream>
#include <string>
#include <cstring>
#include <sys/socket.h>

Connection::Connection(int fd) : fd(fd) {}

Connection::~Connection() {
    close(fd);
}

int Connection::getFd() const {
    return fd;
}

bool Connection::handleRead() {
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
                    processRequest();
                    readBuffer.clear();
                }
                return true;
            }
            std::cerr << "Read error: " << strerror(errno) << "\n";
            return false;
        }
        else if(bytesRead == 0) {
            return false; // Client disconnected
        }
    }
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