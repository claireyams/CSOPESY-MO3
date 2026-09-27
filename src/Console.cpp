#include "Console.h"
#include <algorithm>
#include <charconv>
#include <iostream>
#include <sstream>
#include <thread>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#endif
#else
#include <sys/ioctl.h>
#include <unistd.h>
#endif

namespace {
// marquee (24 rows) + 6 output rows + 1 prompt row + 1 spare row
constexpr int MIN_ROWS = 32;
constexpr int RESIZE_POLL_MS = 100;

// ANSI: move the cursor to a row (1-based)
std::string at(int row) { return "\033[" + std::to_string(row) + ";1H"; }
}

// ---- screen layout helpers (added) ----

void Console::enableAnsi() const {
#ifdef _WIN32
    HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode = 0;
    if (out != INVALID_HANDLE_VALUE && GetConsoleMode(out, &mode)) {
        SetConsoleMode(out, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    }
#endif
}

Console::Size Console::queryTerminalSize() const {
#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO info;
    if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info)) {
        return { info.srWindow.Bottom - info.srWindow.Top + 1,
                 info.srWindow.Right - info.srWindow.Left + 1 };
    }
#else
    winsize ws{};
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0 && ws.ws_row > 0) {
        return { ws.ws_row, ws.ws_col };
    }
#endif
    return { 30, 120 };
}

void Console::updateLayout() {
    Size size = queryTerminalSize();
    realRows = size.rows;
    realCols = size.cols;
    termRows = std::max(size.rows, MIN_ROWS);
    promptRow = termRows - 1;              // last row stays free so pressing Enter never scrolls
    outputRow = marquee->height() + 1;
}

// Apply a resize only between input lines, when no partially typed text can be lost.
void Console::checkResize() {
    Size size = queryTerminalSize();
    if (size.rows == realRows && size.cols == realCols) return;
    updateLayout();
}

// Put the cursor on the (cleared) prompt row at the bottom.
void Console::moveToPrompt() const {
    std::cout << at(promptRow) << "\033[2K";
}

// Clear the output area under the marquee and put the cursor at its top,
// so the normal std::cout messages appear there.
void Console::moveToOutput() const {
    for (int r = outputRow; r < promptRow; r++) std::cout << at(r) << "\033[2K";
    std::cout << at(outputRow);
    if (realRows < MIN_ROWS) {
        std::cout << "Window too small - enlarge it to at least " << MIN_ROWS << " rows.\n";
    }
}

// Marquee stopped: wipe the marquee area and show the welcome banner again.
void Console::showIdleScreen() {
    for (int r = 1; r < outputRow; r++) std::cout << at(r) << "\033[2K";
    std::cout << "\033[H";
    printWelcome();
    moveToOutput();
}

void Console::printWelcome() const {
    std::cout << "==============================\n"
              << "     Welcome to CSOPESY!\n"
              << "==============================\n"
              << "Group developers:\nCHIU, Kristopher Lance,\nKE, Xan Luo,\nRAMIREZ, Diana Angela,\nYAMSUAN, Rhian Claire\n"
              << "Version date: 2026-09-21\n\n"
              << "Type help to see the available commands.\n\n";
}

void Console::printHelp() const {
    std::cout << "help          Show commands\n"
              << "start_marquee Start animation\n"
              << "stop_marquee  Stop animation\n"
              << "set_text <text>      Set marquee text\n"
              << "set_speed <ms>       Set refresh (ms)\n"
              << "exit          Terminate console\n";
}

void Console::run() {
    marquee = std::make_unique<Marquee>();
    enableAnsi();
    updateLayout();
    std::cout << "\033[2J\033[3J\033[H";   // clear the console, then show only the welcome banner
    printWelcome();
    
    std::thread animThread(&Console::animationLoop, this);
    
    std::string line;
    while (true) {
        checkResize();
        moveToPrompt();
        std::cout << "Command> ";
        if (!std::getline(std::cin, line)) break;
        checkResize();
        moveToOutput();
        if (!handleCommand(line)) break;
    }
    
    running = false;
    animationWake.notify_one();
    animThread.join();
    std::cout << at(termRows) << "\n";   // leave the shell prompt below our screen
}

void Console::animationLoop() {
    while (running) {
        std::uint64_t observedRevision;
        {
            std::lock_guard<std::mutex> lock(animationWaitMutex);
            observedRevision = animationRevision;
        }

        // Draw only when the marquee fits, without touching the active prompt.
        const Size size = queryTerminalSize();
        const bool frameFits = size.rows >= marquee->height() + 2 &&
                               size.cols > marquee->width();
        if (frameFits && marquee->isActive()) {
            marquee->update();
            marquee->render();
        }
        
        // Wake early when start or speed changes, even if the old interval is long.
        std::unique_lock<std::mutex> lock(animationWaitMutex);
        const int waitMs = frameFits ? refreshMs.load()
                                     : std::min(refreshMs.load(), RESIZE_POLL_MS);
        animationWake.wait_for(
            lock,
            std::chrono::milliseconds(waitMs),
            [this, observedRevision] {
                return !running.load() || animationRevision != observedRevision;
            }
        );
    }
}

bool Console::handleCommand(const std::string& line) {
    std::istringstream input(line);
    std::string command;
    input >> command;

    if (command == "help") {
        printHelp();
    } else if (command == "start_marquee") {
        marquee->start();
        {
            std::lock_guard<std::mutex> lock(animationWaitMutex);
            ++animationRevision;
        }
        animationWake.notify_one();
        std::cout << "Marquee started.\n";
    } else if (command == "stop_marquee") {
        marquee->stop();
        showIdleScreen();
        std::cout << "Marquee stopped.\n";
    } else if (command == "set_text") {
        std::string text;
        std::getline(input, text);
        text.erase(0, text.find_first_not_of(" \t"));
        if (!text.empty()) {
            marquee->setText(text);
            std::cout << "Text set to: " << text << "\n";
        } else {
            std::cout << "Usage: set_text <text>\n";
        }
    } else if (command == "set_speed") {
        std::string argument;
        std::getline(input, argument);
        const auto first = argument.find_first_not_of(" \t");
        const auto last = argument.find_last_not_of(" \t");
        int ms = 0;
        bool valid = false;
        if (first != std::string::npos) {
            const char* begin = argument.data() + first;
            const char* end = argument.data() + last + 1;
            const auto result = std::from_chars(begin, end, ms);
            valid = result.ec == std::errc{} && result.ptr == end && ms > 0;
        }
        if (valid) {
            marquee->setSpeed(ms);
            {
                std::lock_guard<std::mutex> lock(animationWaitMutex);
                refreshMs = ms;
                ++animationRevision;
            }
            animationWake.notify_one();
            std::cout << "Speed set to " << ms << "ms.\n";
        } else {
            std::cout << "Usage: set_speed <positive ms>\n";
        }
    } else if (command == "exit") {
        std::cout << "Goodbye!\n";
        return false;
    } else if (!command.empty()) {
        std::cout << "Unknown command. Type help.\n";
    }
    return true;
}
