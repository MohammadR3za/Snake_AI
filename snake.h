#ifndef SNAKE_H
#define SNAKE_H

#include <vector>
#include <deque>
#include <random>
#include <string>
#include <chrono>
#include <thread>
#include <fstream>
#include <iostream>
#include <algorithm>
#include "nn.h"

// Game modes
enum class GameMode {
    MANUAL,
    AI,
    TRAINING
};

// Directions
enum class Direction {
    UP,
    RIGHT,
    DOWN,
    LEFT,
    NONE
};

// Point structure
struct Point {
    int x;
    int y;
    
    Point(int x = 0, int y = 0) : x(x), y(y) {}
    
    Point operator+(const Point& other) const {
        return Point(x + other.x, y + other.y);
    }
    
    bool operator==(const Point& other) const {
        return x == other.x && y == other.y;
    }
    
    bool operator!=(const Point& other) const {
        return !(*this == other);
    }
};

// Reward values - optimized based on research
namespace Reward {
    constexpr double DEATH = -10.0;
    constexpr double SURVIVAL = 0.01;
    constexpr double EAT_FOOD = 10.0;
    constexpr double MOVE_CLOSER_TO_FOOD = 0.1;
    constexpr double MOVE_AWAY_FROM_FOOD = -0.1;
}

// Experience structure for replay memory
struct Experience {
    std::vector<double> state;
    int action;
    double reward;
    std::vector<double> next_state;
    bool done;
};

class Snake {
public:
    Snake(unsigned int width = 20, unsigned int height = 15, GameMode mode = GameMode::MANUAL);
    
    void reset();
    bool move(Direction dir);
    void play();
    void train(int episodes, int max_steps = 1000);
    
    bool load_model(const std::string& filename);
    bool save_model(const std::string& filename);
    bool load_config(const std::string& filename);
    
private:
    // Game state
    unsigned int width;
    unsigned int height;
    std::deque<Point> segments;
    Point food;
    Direction current_direction;
    int score;
    int highest_score;
    bool game_over;
    int steps_since_last_food;
    int max_steps_without_food;
    
    // Game settings
    GameMode game_mode;
    bool visualize;
    int animation_delay_ms;
    
    // AI components
    NeuralNetwork nn;
    NeuralNetwork target_nn;
    double exploration_rate;
    double exploration_min;
    double exploration_decay;
    size_t replay_buffer_size;
    size_t min_replay_size;
    size_t batch_size;
    double discount_factor;
    int target_update_frequency;
    int current_iteration;
    std::deque<Experience> replay_buffer;
    std::mt19937 rng;
    
    // Helper methods
    void create_food();
    void render() const;
    Direction get_user_input() const;
    std::vector<double> get_state() const;
    std::vector<Direction> get_valid_directions() const;
    bool will_collide(Direction dir) const;
    Point direction_to_delta(Direction dir) const;
    double calculate_distance(const Point& p1, const Point& p2) const;
    void add_to_replay_memory(Experience&& exp);
    void train_from_replay_memory();
    Direction choose_action(const std::vector<double>& state, bool explore);
};

#endif // SNAKE_H
