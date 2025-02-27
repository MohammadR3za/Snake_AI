#ifndef SNAKE_H
#define SNAKE_H

#include "common.h"
#include "point.h"
#include "neural_network.h"
#include <vector>
#include <memory>
#include <random>

enum class Direction {
    UP,
    RIGHT,
    DOWN,
    LEFT
};

class Snake {
public:
    Snake(int grid_width, int grid_height);
    
    void update();
    bool is_game_over() const;
    double calculate_fitness() const;
    void set_direction(Direction dir);
    void set_network(std::unique_ptr<NeuralNetwork> net);
    
    int get_score() const { return score; }
    const Point& get_food() const { return food; }
    const std::vector<Point>& get_body() const { return snake_body; }
    
private:
    void place_food();
    void grow_body();
    bool check_collision() const;
    Direction get_ai_direction();
    std::vector<double> get_network_inputs() const;
    
    Direction direction = Direction::RIGHT;
    int grid_width;
    int grid_height;
    int score = 0;
    int steps_taken = 0;
    int steps_since_last_food = 0;
    int max_steps_without_food;
    
    std::vector<Point> snake_body;
    Point food;
    
    std::unique_ptr<NeuralNetwork> network;
    std::mt19937 rng;
    
    // Stats for fitness calculation
    double efficiency_factor = 0.0;
    double exploration_factor = 0.0;
    int cells_visited = 0;
    std::vector<std::vector<bool>> visited_cells;
};

#endif // SNAKE_H
