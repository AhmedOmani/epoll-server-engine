#pragma once
#include <string>

class Connection {
private:
    int fd;
    std::string readBuffer;
    
    void processRequest();
    void sendResponse(int contentLength , const std::string& response) ;

public:
    Connection(int fd);
    ~Connection();

    int getFd() const;
    bool handleRead(); // return false if the client disconnected
};