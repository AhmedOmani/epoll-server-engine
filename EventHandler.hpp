#pragma once

class EventHandler {
public:
    virtual int getFd() const = 0;
    virtual void handleEvent() = 0;
    virtual ~EventHandler() = default;
};