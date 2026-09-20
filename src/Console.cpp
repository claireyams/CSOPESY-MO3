#include "Console.h"
#include <iostream>
#include <sstream>
#include <thread>

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
              << "start_marquee        Start the marquee animation\n"
              << "stop_marquee         Stop the marquee animation\n"
              << "set_text             Set the marquee text\n"
              << "set_speed            Set the animation refresh interval\n"
              << "exit                 Terminate the console\n\n";
}

void Console::run() {
    printWelcome();
    marquee = std::make_unique<Marquee>();
    
    std::thread animThread(&Console::animationLoop, this);
    
    std::string line;
    while (true) {
        std::cout << "Command>> ";
        if (!std::getline(std::cin, line)) break;
        if (!handleCommand(line)) break;
    }
    
    running = false;
    animThread.join();
}

void Console::animationLoop() {
    while (running) {
        if (marquee->isActive()) {
            marquee->update();
            marquee->render();
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(refreshMs));
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
        std::cout << "Marquee started.\n";
    } else if (command == "stop_marquee") {
        marquee->stop();
        std::cout << "Marquee stopped.\n";
    } else if (command == "set_text") {
        std::string text;
        std::cout << "Enter text: ";
        std::getline(std::cin, text);
        if (!text.empty()) {
            marquee->setText(text);
            std::cout << "Text set to: " << text << "\n";
        } else {
            std::cout << "No text provided.\n";
        }
    } else if (command == "set_speed") {
        int ms;
        std::cout << "Enter speed (ms): ";
        std::cin >> ms;
        std::cin.ignore();
        if (ms > 0) {
            marquee->setSpeed(ms);
            refreshMs = ms;
            std::cout << "Speed set to " << ms << "ms.\n";
        } else {
            std::cout << "Error: Speed must be positive.\n";
        }
    } else if (command == "exit") {
        std::cout << "Goodbye!\n";
        return false;
    } else if (!command.empty()) {
        std::cout << "Unknown command. Type help.\n";
    }
    return true;
}