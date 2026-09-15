#pragma once

#include <string>

class Console {
public:
    void run();

private:
    void printWelcome() const;
    void printHelp() const;
    bool handleCommand(const std::string& line);
};