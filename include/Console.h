#pragma once

#include <string>

class Console {
public:
    void run();

private:
    std::string marqueeText_;

    void printWelcome() const;
    void printHelp() const;
    bool handleCommand(const std::string& line);
    bool handleHelp(const std::string& args);
    bool handleSetText(const std::string& args);
};
