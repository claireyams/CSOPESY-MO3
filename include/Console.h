#pragma once
#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <string>
#include <memory>
#include <chrono>
#include <mutex>
#include "Marquee.h"

class Console {
public:
    void run();

private:
    void printWelcome() const;
    void printHelp() const;
    bool handleCommand(const std::string& line);
    void animationLoop();

    // Screen layout: marquee on top, command output under it, prompt near the bottom.
    struct Size { int rows; int cols; };
    void enableAnsi() const;
    Size queryTerminalSize() const;
    void updateLayout();
    void checkResize();
    void moveToPrompt() const;
    void moveToOutput() const;
    void showIdleScreen();
    
    std::unique_ptr<Marquee> marquee;
    std::atomic<bool> running{true};
    std::atomic<int> refreshMs{100};
    std::condition_variable animationWake;
    std::mutex animationWaitMutex;
    std::uint64_t animationRevision{0};

    std::atomic<int> realRows{30}, realCols{120};   // actual terminal size
    std::atomic<int> termRows{30};                  // rows used for the layout
    std::atomic<int> promptRow{29};
    std::atomic<int> outputRow{25};                 // first row of the output area
};
