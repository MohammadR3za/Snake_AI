
// game.h
#ifndef GAME_H
#define GAME_H

#include "snake.h"
#include "food.h"
#include "direction.h"
#include <vector>

class Game {
public:
    Game(int width = 20, int height = 20, int num_snakes = 1);
    ~Game();
    
    void reset();
    bool update(Direction dir); // For single snake (backward compatibility)
    bool update(const std::vector<Direction>& directions); // For multiple snakes
    
    // Getters
    int get_width() const { return width; }
    int get_height() const { return height; }
    int get_score(int snake_idx = 0) const; // Modified to support multiple snakes
    bool is_game_over() const { return game_over; }
    const Snake& get_snake(int snake_idx = 0) const; // Modified to support multiple snakes
    const Food& get_food() const { return food; }
    int get_steps() const { return steps; }
    int get_steps_without_food(int snake_idx = 0) const; // Modified for multiple snakes
    int get_population_size() const { return snakes.size(); }
    bool is_snake_alive(int idx) const;
    std::vector<int> get_top_scores(int count) const;
    
    // Added methods needed by other components
    void set_max_steps_without_food(int max) { max_steps_without_food = max; }
    void set_generation(int gen) { generation = gen; }
    int get_generation() const { return generation; }
    std::vector<double> get_state_for_ai(int snake_idx = 0) const;
    std::vector<std::vector<double>> get_states_for_ai() const;
    
private:
    int width;
    int height;
    std::vector<Snake> snakes;           // Multiple snakes
    std::vector<bool> snakes_alive;      // Track which snakes are still alive
    std::vector<int> scores;             // Score for each snake
    std::vector<int> steps_without_food; // Steps without food for each snake
    Food food;
    bool game_over;
    int steps;
    int max_steps_without_food;
    int generation;
    
    void spawn_food();
};

#endif // GAME_H
