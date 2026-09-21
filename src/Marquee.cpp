#include "Marquee.h"
#include <algorithm>
#include <iostream>
#include <sstream>

namespace {
void drawText(std::vector<std::string>& canvas, int row, int col,
              const std::string& value,
              std::vector<std::string>& colors, char color) {
    if (row < 0 || row >= static_cast<int>(canvas.size())) return;

    for (int i = 0; i < static_cast<int>(value.length()); ++i) {
        const int x = col + i;
        if (x >= 0 && x < static_cast<int>(canvas[row].length())) {
            canvas[row][x] = value[i];
            colors[row][x] = color;
        }
    }
}

const char* colorCode(char color) {
    switch (color) {
        case 'B': return "\033[96m"; // border: bright cyan
        case 'C': return "\033[97m"; // clouds: bright white
        case 'G': return "\033[33m"; // ground: yellow/brown
        case 'P': return "\033[92m"; // pipes: bright green
        case 'S': return "\033[93m"; // snake: bright yellow
        default:  return "\033[0m";
    }
}
}

Marquee::Marquee() : text("CSOPESY"), y(10), vy(0), speed(100),
                     isRunning(false), frameNumber(0), nextGapVariant(0) {
    generateObstacles();
    resetTrail();
}

void Marquee::generateObstacles() {
    obstacles.clear();
    for (int i = 0; i < 4; ++i) {
        Obstacle obs;
        obs.x = 45 + i * 24;
        obs.gapY = 4 + (i % 3) * 5;
        obs.gapHeight = 7;
        obstacles.push_back(obs);
    }
    nextGapVariant = 1;
}

void Marquee::resetTrail() {
    trailY.assign(std::max<std::size_t>(text.length(), 1), y);
}

int Marquee::snakeHeadX() const {
    // keep most of the text visible while leaving room for incoming obstacles
    return std::clamp(static_cast<int>(text.length()) + 8, 20, SCREEN_WIDTH - 15);
}

void Marquee::update() {
    std::lock_guard<std::mutex> lock(mtx);
    if (!isRunning) return;

    ++frameNumber;

    // pipes are the foreground layer and move one cell per frame
    for (auto& obs : obstacles) --obs.x;

    for (auto& obs : obstacles) {
        if (obs.x + OBSTACLE_WIDTH < 0) {
            int rightmostX = 0;
            for (const auto& other : obstacles) {
                rightmostX = std::max(rightmostX, other.x);
            }
            obs.x = rightmostX + 24;
            obs.gapY = 4 + (nextGapVariant % 3) * 5;
            nextGapVariant = (nextGapVariant + 1) % 3;
        }
    }

    // autopilot the snake head toward the next pipe opening
    const int headX = snakeHeadX();
    const Obstacle* nextObstacle = nullptr;
    for (const auto& obs : obstacles) {
        if (obs.x + OBSTACLE_WIDTH < headX) continue;
        if (nextObstacle == nullptr || obs.x < nextObstacle->x) {
            nextObstacle = &obs;
        }
    }

    if (nextObstacle != nullptr) {
        const int targetY = nextObstacle->gapY + nextObstacle->gapHeight / 2;
        vy = (y < targetY) ? 1 : (y > targetY) ? -1 : 0;
    }

    y += vy;
    y = std::clamp(y, 2, SCREEN_HEIGHT - 4);

    // each character uses an older head position (making a snake-like trail)
    trailY.insert(trailY.begin(), y);
    trailY.resize(std::max<std::size_t>(text.length(), 1), y);
}

void Marquee::render() const {
    std::lock_guard<std::mutex> lock(mtx);
    if (!isRunning) return;

    std::vector<std::string> canvas(
        SCREEN_HEIGHT, std::string(SCREEN_WIDTH, ' ')
    );
    std::vector<std::string> colors(
        SCREEN_HEIGHT, std::string(SCREEN_WIDTH, ' ')
    );

    // far clouds move slowly.
    const int cloudOffset = static_cast<int>((frameNumber / 4) % SCREEN_WIDTH);
    const int cloudPositions[] = {6, 38, 68};
    const int cloudRows[] = {3, 8, 5};
    for (int i = 0; i < 3; ++i) {
        int cloudX = (cloudPositions[i] - cloudOffset + SCREEN_WIDTH) % SCREEN_WIDTH;
        drawText(canvas, cloudRows[i], cloudX,     "    .--.    ", colors, 'C');
        drawText(canvas, cloudRows[i] + 1, cloudX, " .-(    ).  ", colors, 'C');
        drawText(canvas, cloudRows[i] + 2, cloudX, "(___.__)__) ", colors, 'C');
    }

    // the ground is the nearest background layer and moves every frame
    const int groundOffset = static_cast<int>(frameNumber % 4);
    for (int col = 0; col < SCREEN_WIDTH; ++col) {
        canvas[SCREEN_HEIGHT - 2][col] = "_.._"[(col + groundOffset) % 4];
        colors[SCREEN_HEIGHT - 2][col] = 'G';
    }

    // draw moving pipes over the background
    for (const auto& obs : obstacles) {
        for (int row = 1; row < SCREEN_HEIGHT - 2; ++row) {
            if (row >= obs.gapY && row < obs.gapY + obs.gapHeight) continue;
            for (int width = 0; width < OBSTACLE_WIDTH; ++width) {
                const int col = obs.x + width;
                if (col >= 0 && col < SCREEN_WIDTH) {
                    canvas[row][col] = '#';
                    colors[row][col] = 'P';
                }
            }
        }
    }

    // last character is the head and earlier characters follow its old path
    const int headX = snakeHeadX();
    for (int i = 0; i < static_cast<int>(text.length()); ++i) {
        const int distanceFromHead = static_cast<int>(text.length()) - 1 - i;
        const int col = headX - distanceFromHead;
        const int row = distanceFromHead < static_cast<int>(trailY.size())
            ? trailY[distanceFromHead]
            : y;
        if (row > 0 && row < SCREEN_HEIGHT - 2 && col >= 0 && col < SCREEN_WIDTH) {
            canvas[row][col] = text[i];
            colors[row][col] = 'S';
        }
    }

    for (int col = 0; col < SCREEN_WIDTH; ++col) {
        canvas.front()[col] = '=';
        canvas.back()[col] = '=';
        colors.front()[col] = 'B';
        colors.back()[col] = 'B';
    }

    // Build the whole frame first and write it in one go (no flicker).
    std::ostringstream frame;

    // Hide cursor, save where it is (the command prompt), and draw from the top-left
    // without clearing the screen. The cursor is put back at the end.
    frame << "\033[?25l" << "\033" "7" << "\033[H";
    
    for (int row = 0; row < SCREEN_HEIGHT; ++row) {
        frame << "\033[" << row + 1 << ";1H";
        char activeColor = '\0';
        for (int col = 0; col < SCREEN_WIDTH; ++col) {
            if (colors[row][col] != activeColor) {
                activeColor = colors[row][col];
                frame << colorCode(activeColor);
            }
            frame << canvas[row][col];
        }
        frame << "\033[0m";
    }

    frame << "\033" "8" << "\033[?25h";   // restore cursor
    std::cout << frame.str() << std::flush;
}

void Marquee::setText(const std::string& newText) {
    std::lock_guard<std::mutex> lock(mtx);
    text = newText;
    resetTrail();
}

void Marquee::setSpeed(int ms) {
    std::lock_guard<std::mutex> lock(mtx);
    speed = ms;
}

void Marquee::start() {
    std::lock_guard<std::mutex> lock(mtx);
    isRunning = true;
    y = 10;
    vy = 0;
    frameNumber = 0;
    generateObstacles();
    resetTrail();
}

void Marquee::stop() {
    std::lock_guard<std::mutex> lock(mtx);
    isRunning = false;
}
