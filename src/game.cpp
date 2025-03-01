
// game.cpp
#include "game.h"
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <cmath>

Game::Game(int width, int height, int num_snakes) 
    : width(width), height(height), 
      game_over(false), 
      steps(0), max_steps_without_food(100),
      generation(0) {
    
    srand(time(nullptr));
    
    // Initialize snakes
    for (int i = 0; i < num_snakes; i++) {
        snakes.push_back(Snake(width/2, height/2));
        snakes_alive.push_back(true);
        scores.push_back(0);
        steps_without_food.push_back(0);
    }
    
    spawn_food();
}

Game::~Game() {}

void Game::reset() {
    // Reinitialize all snakes
    for (size_t i = 0; i < snakes.size(); i++) {
        snakes[i] = Snake(width/2, height/2);
        snakes_alive[i] = true;
        scores[i] = 0;
        steps_without_food[i] = 0;
    }
    
    game_over = false;
    steps = 0;
    
    spawn_food();
}

bool Game::update(Direction dir) {
    // For backward compatibility - update only the first snake
    std::vector<Direction> directions(1, dir);
    return update(directions);
}

bool Game::update(const std::vector<Direction>& directions) {
    if (game_over) return false;
    
    steps++;
    bool any_snake_alive = false;
    
    for (size_t i = 0; i < snakes.size(); i++) {
        if (!snakes_alive[i]) continue;  // Skip dead snakes
        
        steps_without_food[i]++;
        
        // Check if max steps without food reached
        if (max_steps_without_food > 0 && steps_without_food[i] >= max_steps_without_food) {
            snakes_alive[i] = false;
            continue;
        }
        
        // Get current head position before moving
        std::pair<int, int> old_head = snakes[i].get_head();
        
        // Move snake
        Direction dir = i < directions.size() ? directions[i] : directions.back();
        snakes[i].move(dir, false);
        
        // Get new head position
        std::pair<int, int> new_head = snakes[i].get_head();
        
        // Check wall collision
        if (new_head.first < 0 || new_head.first >= width || 
            new_head.second < 0 || new_head.second >= height) {
            snakes_alive[i] = false;
            continue;
        }
        
        // Check self collision
        if (snakes[i].check_collision_with_self()) {
            snakes_alive[i] = false;
            continue;
        }
        
        // At least one snake is still alive
        any_snake_alive = true;
        
        // Check food collision
        if (new_head.first == food.get_x() && new_head.second == food.get_y()) {
            // Add a segment to the snake (grow)
            snakes[i].move(dir, true);
            
            // Reset steps without food counter
            steps_without_food[i] = 0;
            
            // Increase score
            scores[i]++;
            
            // Spawn new food
            spawn_food();
        }
    }
    
    // If all snakes are dead, game over
    if (!any_snake_alive) {
        game_over = true;
        return false;
    }
    
    return true;
}

void Game::spawn_food() {
    int food_x, food_y;
    bool collision;
    
    do {
        food_x = rand() % width;
        food_y = rand() % height;
        
        collision = false;
        for (size_t i = 0; i < snakes.size(); i++) {
            if (snakes_alive[i] && snakes[i].check_collision_with_point(food_x, food_y)) {
                collision = true;
                break;
            }
        }
    } while (collision);
    
    food.set_position(food_x, food_y);
}

int Game::get_score(int snake_idx) const {
    if (snake_idx >= 0 && snake_idx < static_cast<int>(scores.size())) {
        return scores[snake_idx];
    }
    return 0;
}

const Snake& Game::get_snake(int snake_idx) const {
    if (snake_idx >= 0 && snake_idx < static_cast<int>(snakes.size())) {
        return snakes[snake_idx];
    }
    return snakes[0]; // Default to first snake if invalid index
}

int Game::get_steps_without_food(int snake_idx) const {
    if (snake_idx >= 0 && snake_idx < static_cast<int>(steps_without_food.size())) {
        return steps_without_food[snake_idx];
    }
    return 0;
}

bool Game::is_snake_alive(int idx) const {
    if (idx >= 0 && idx < static_cast<int>(snakes_alive.size())) {
        return snakes_alive[idx];
    }
    return false;
}

std::vector<int> Game::get_top_scores(int count) const {
    std::vector<int> sorted_scores = scores;
    std::sort(sorted_scores.begin(), sorted_scores.end(), std::greater<int>());
    
    // Return top scores
    std::vector<int> top;
    for (int i = 0; i < count && i < static_cast<int>(sorted_scores.size()); i++) {
        top.push_back(sorted_scores[i]);
    }
    return top;
}

std::vector<double> Game::get_state_for_ai(int snake_idx) const {
    if (snake_idx < 0 || snake_idx >= static_cast<int>(snakes.size()) || !snakes_alive[snake_idx]) {
        return std::vector<double>();
    }
    
    std::vector<double> state;
    
    // Current direction (one-hot encoding)
    Direction current_direction = snakes[snake_idx].get_direction();
    for (int i = 0; i < 4; i++) {
        state.push_back(i == direction_to_int(current_direction) ? 1.0 : 0.0);
    }
    
    // Vision in 8 directions (distance to wall, food, and self)
    const int directions[8][2] = {
        {0, -1},  // Up
        {1, -1},  // Up-Right
        {1, 0},   // Right
        {1, 1},   // Down-Right
        {0, 1},   // Down
        {-1, 1},  // Down-Left
        {-1, 0},  // Left
        {-1, -1}  // Up-Left
    };
    
    std::pair<int, int> head = snakes[snake_idx].get_head();
    
    for (int i = 0; i < 8; i++) {
        double wall_distance = 1.0;
        double food_distance = 0.0;
        double self_distance = 0.0;
        bool food_found = false;
        bool self_found = false;
        
        int x = head.first;
        int y = head.second;
        int steps = 0;
        
        const auto& snake_body = snakes[snake_idx].get_body();
        
        while (true) {
            x += directions[i][0];
            y += directions[i][1];
            steps++;
            
            // Check if out of bounds
            if (x < 0 || x >= width || y < 0 || y >= height) {
                wall_distance = 1.0 / steps;
                break;
            }
            
            // Check if food found
            if (!food_found && x == food.get_x() && y == food.get_y()) {
                food_distance = 1.0 / steps;
                food_found = true;
            }
            
            // Check if self found
            if (!self_found) {
                for (size_t j = 1; j < snake_body.size(); j++) {
                    if (x == snake_body[j].first && y == snake_body[j].second) {
                        self_distance = 1.0 / steps;
                        self_found = true;
                        break;
                    }
                }
            }
            
            // If we've found everything, no need to continue
            if (food_found && self_found) {
                break;
            }
        }
        
        state.push_back(wall_distance);
        state.push_back(food_distance);
        state.push_back(self_distance);
    }
    
    return state;
}

std::vector<std::vector<double>> Game::get_states_for_ai() const {
    std::vector<std::vector<double>> states;
    
    for (size_t i = 0; i < snakes.size(); i++) {
        if (snakes_alive[i]) {
            states.push_back(get_state_for_ai(i));
        }
    }
    
    return states;
}
