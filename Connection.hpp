#pragma once
#include "EventHandler.hpp"
#include <string>
#include <ctime>
#include <chrono>
#include <functional>

class Connection : public EventHandler {
private:
    int fd;
    std::chrono::system_clock::time_point lastActive;
    std::string readBuffer;
    std::function<void(int)> onCloseCallback;
    std::function<void(int)> onReadCallback;
    
    void processRequest();
    void sendResponse(int contentLength , const std::string& response) ;

public:
    Connection(int fd , std::chrono::system_clock::time_point lastActive , std::function<void(int)> onCloseCallback, std::function<void(int)> onReadCallback);
    ~Connection();

    int getFd() const override;
    std::chrono::system_clock::time_point getLastActiveTime();
    void setLastActiveTime(std::chrono::system_clock::time_point lastActive);
    void handleRead(); // return false if the client disconnected
    void handleEvent() override;
};