#include "Marquee.h"
#include <iostream>
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
    std::cout << "\033[2J\033[H";
    
    for (int i = 0; i < SCREEN_WIDTH; i++) std::cout << "=";
    std::cout << "\n";
    
    for (int row = 1; row < SCREEN_HEIGHT - 1; row++) {
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
            
            std::cout << cell;
        }
        std::cout << "\n";
    }
    
    for (int i = 0; i < SCREEN_WIDTH; i++) std::cout << "=";
}

void Marquee::setText(const std::string& newText) {
    text = newText;
}

void Marquee::setSpeed(int ms) {
    speed = ms;
}

void Marquee::start() {
    isRunning = true;
    x = 10;
    y = 10;
    vx = 1;
    vy = 0;
}

void Marquee::stop() {
    isRunning = false;
}