#pragma once
#include <string>
#include <vector>

struct Obstacle {
    int x;
    int gapY;
    int gapHeight = 6;
};

class Marquee {
private:
    std::string text;
    int x, y;
    int vx, vy;
    int speed;
    bool isRunning;
    
    std::vector<Obstacle> obstacles;
    
    const int GRAVITY = 1;
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
    bool isActive() const { return isRunning; }
    
private:
    void generateObstacles();
};