#ifndef SNAKE_HH
#define SNAKE_HH

#include <vector>
#include "nn.h" // Assuming NeuralNetwork class is defined here

class Point {
public:
    Point() : x(0), y(0) {}
    Point(int nx, int ny) : x(nx), y(ny) {}
    Point(Point &&) = default;
    Point(const Point &) = default;
    Point &operator=(Point &&) = default;
    Point &operator=(const Point &) = default;
    ~Point() = default;

    int x, y;
    bool operator==(const Point& rvalue) const { return x == rvalue.x && y == rvalue.y; }
};

// Note: Removed GameState class since Snake maintains its own state
// If needed later, it can be reintroduced

class Snake {
public:
    Snake(uint width, uint height);
    Snake(Snake &&) = default;
    Snake(const Snake &) = default;
    Snake &operator=(Snake &&) = default;
    Snake &operator=(const Snake &) = default;
    ~Snake();

    // Neural network-related methods
    std::vector<double> get_inputs(); // Updated to use internal state
    int choose_action(); // Moved into class
    std::vector<double> action_to_target(int action);
    void collect_training_data(std::vector<std::vector<double>>& inputs,
                              std::vector<std::vector<double>>& targets, int num_games);
    void train(int num_games, int epochs);
    void play();

    // Game logic methods
    void drawArea();
    bool move(); // Modified to return bool for collision
    void createFood();
    void reset();
    bool hasCollision();
    void gameOver();

private:
    bool **area;
    std::vector<Point> segments;
    Point lastSegment;
    uint width, height;
    Point head, food;
    uint speed, direction; // 0: up, 1: down, 2: right, 3: left
    bool grow;
    uint score, highestScore, playedGames;
    bool isGameOver;
    bool isStartOfGame;
    NeuralNetwork nn; // Neural network member

    void clearArea();
};

#endif // SNAKE_HH
