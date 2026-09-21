#include "Marquee.h"
#include <iostream>
#include <sstream>
#include <cmath>

Marquee::Marquee() : text("CSOPESY"), x(10), y(10), vx(1), vy(0), 
                     speed(100), isRunning(false) {
    generateObstacles();
}

void Marquee::generateObstacles() {
    obstacles.clear();
    for (int i = 1; i < 6; i++) {
        Obstacle obs;
        obs.x = i * 15;
        obs.gapY = 8 + (i % 3) * 2;
        obs.gapHeight = 6;
        obstacles.push_back(obs);
    }
}

void Marquee::update() {
    std::lock_guard<std::mutex> lock(mtx);
    if (!isRunning) return;
    
    x += vx;
    
    // Autopilot: find next obstacle and navigate through its gap
    for (const auto& obs : obstacles) {
        if (x + 10 >= obs.x && x < obs.x + OBSTACLE_WIDTH) {
            // text is at this obstacle - aim for center of gap
            int gapCenter = obs.gapY + obs.gapHeight / 2;
            
            if (y < gapCenter - 1) {
                vy = 1;  // Move down
            } else if (y > gapCenter + 1) {
                vy = -1;  // Move up
            } else {
                vy = 0;  // Center aligned
            }
            break;
        }
    }
    
    y += vy;
    
    // Soft boundary constraints
    if (y < 1) y = 1;
    if (y > SCREEN_HEIGHT - 3) y = SCREEN_HEIGHT - 3;
    
    // Reset when reaching the end
    if (x > SCREEN_WIDTH) {
        x = 10;
        y = 10;
    }
}

void Marquee::render() const {
    std::lock_guard<std::mutex> lock(mtx);
    if (!isRunning) return;

    // Build the whole frame first and write it in one go (no flicker).
    std::ostringstream frame;

    // Hide cursor, save where it is (the command prompt), and draw from the top-left
    // without clearing the screen. The cursor is put back at the end.
    frame << "\033[?25l" << "\033" "7" << "\033[H";
    
    for (int i = 0; i < SCREEN_WIDTH; i++) frame << "=";
    
    for (int row = 1; row < SCREEN_HEIGHT - 1; row++) {
        frame << "\033[" << row + 1 << ";1H";   // jump to the row (instead of "\n")
        for (int col = 0; col < SCREEN_WIDTH; col++) {
            char cell = ' ';
            
            if (row == y && col >= x && col < x + (int)text.length()) {
                cell = text[col - x];
            }
            
            if (cell == ' ') {
                for (const auto& obs : obstacles) {
                    if (col >= obs.x && col < obs.x + OBSTACLE_WIDTH) {
                        if (row < obs.gapY || row >= obs.gapY + obs.gapHeight) {
                            cell = '#';
                            break;
                        }
                    }
                }
            }
            
            frame << cell;
        }
    }
    
    frame << "\033[" << SCREEN_HEIGHT << ";1H";
    for (int i = 0; i < SCREEN_WIDTH; i++) frame << "=";

    frame << "\033" "8" << "\033[?25h";   // restore cursor
    std::cout << frame.str() << std::flush;
}

void Marquee::setText(const std::string& newText) {
    std::lock_guard<std::mutex> lock(mtx);
    text = newText;
}

void Marquee::setSpeed(int ms) {
    std::lock_guard<std::mutex> lock(mtx);
    speed = ms;
}

void Marquee::start() {
    std::lock_guard<std::mutex> lock(mtx);
    isRunning = true;
    x = 10;
    y = 10;
    vx = 1;
    vy = 0;
}

void Marquee::stop() {
    std::lock_guard<std::mutex> lock(mtx);
    isRunning = false;
}