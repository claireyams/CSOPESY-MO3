#include "Console.h"

#include <iostream>
#include <sstream>

void Console::run() {
    printWelcome();

    // TODO: Replace blocking getline with input polling in the future when animation is implemented.
    std::string line;
    while (std::cout << "Command> " && std::getline(std::cin, line)) {
        if (!handleCommand(line)) {
            break;
        }
    }
}

void Console::printWelcome() const {
    std::cout << "==============================\n"
              << "     Welcome to CSOPESY!\n"
              << "==============================\n"
              << "Group developers:\nCHIU, Kristopher Lance,\nKE, Xan Luo,\nRAMIREZ, Diana Angela,\nYAMSUAN, Rhian Claire\n"
              << "Version date: 2026-09-15\n\n"
              << "Type help to see the available commands.\n\n";
}

void Console::printHelp() const {
    std::cout << "help                 Show available commands\n"
              << "start_marquee        Start the marquee animation (TODO)\n"
              << "stop_marquee         Stop the marquee animation (TODO)\n"
              << "set_text <text>      Set the marquee text (TODO)\n"
              << "set_speed <ms>       Set the animation refresh interval in milliseconds (TODO)\n"
              << "exit                 Terminate the console\n";
}

bool Console::handleCommand(const std::string& line) {
    std::istringstream input(line);
    std::string command;
    input >> command;

    if (command == "help") {
        printHelp();
    } else if (command == "start_marquee") {
        // TODO
    } else if (command == "stop_marquee") {
        // TODO
    } else if (command == "set_text") {
        // TODO
    } else if (command == "set_speed") {
        // TODO
    } else if (command == "exit") {
        std::cout << "Goodbye!\n";
        return false;
    } else if (!command.empty()) {
        std::cout << "Unknown command. Type help.\n";
    }

    return true;
}