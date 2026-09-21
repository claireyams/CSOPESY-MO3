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
              << "Marquee Console Exercise\n"
              << "Group developers:\nCHIU, Kristopher Lance,\nKE, Xan Luo,\nRAMIREZ, Diana Angela,\nYAMSUAN, Rhian Claire\n"
              << "Version date: 2026-09-21\n\n"
              << "Type help to see the available commands.\n\n";
}

void Console::printHelp() const {
    std::cout << "help                 Show available commands\n"
              << "set_text <text>      Set the marquee text\n"
              << "exit                 Terminate the console\n";
}

bool Console::handleCommand(const std::string& line) {
    std::istringstream input(line);
    std::string command;
    input >> command;
    std::string args;
    std::getline(input, args);
    if (!args.empty()) {
        args.erase(0, 1); // removes the separator after the command
    }

    if (command == "help") {
        return handleHelp(args);
    } else if (command == "set_text") {
        return handleSetText(args);
    } else if (command == "exit") {
        std::cout << "Goodbye!\n";
        return false;
    }

    std::cout << "Unknown command. Type help.\n";
    return true;
}

bool Console::handleHelp(const std::string&) {
    printHelp();
    return true;
}

bool Console::handleSetText(const std::string& args) {
    if (args.find_first_not_of(" \t") == std::string::npos) {
        std::cout << "Please provide text after set_text.\n";
    } else {
        marqueeText_ = args;
        std::cout << "Marquee text set to: " << marqueeText_ << '\n';
    }
    return true;
}