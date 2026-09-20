#pragma once
#include <string>
#include <memory>
#include <chrono>
#include "Marquee.h"

class Console {
public:
    void run();

private:
    void printWelcome() const;
    void printHelp() const;
    bool handleCommand(const std::string& line);
    void animationLoop();
    
    std::unique_ptr<Marquee> marquee;
    bool running = true;
    int refreshMs = 100;
};