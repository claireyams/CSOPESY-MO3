#pragma once
#include <string>
#include <vector>
#include <mutex>

struct Obstacle {
    int x;
    int gapY;
    int gapHeight = 6;
};

class Marquee {
private:
    std::string text;
    int y;
    int vy;
    int speed;
    bool isRunning;
    mutable std::mutex mtx;   // update/render run on the animation thread, start/stop/setText on the input thread

    std::vector<Obstacle> obstacles;
    std::vector<int> trailY;
    long long frameNumber;
    int nextGapVariant;

    const int SCREEN_WIDTH = 80;
    const int SCREEN_HEIGHT = 24;
    const int OBSTACLE_WIDTH = 3;

public:
    Marquee();
    void setText(const std::string& newText);
    void setSpeed(int ms);
    void start();
    void stop();
    void update();
    void render() const;
    bool isActive() const { std::lock_guard<std::mutex> lock(mtx); return isRunning; }
    int height() const { return SCREEN_HEIGHT; }
    
private:
    void generateObstacles();
    void resetTrail();
    int snakeHeadX() const;
};
