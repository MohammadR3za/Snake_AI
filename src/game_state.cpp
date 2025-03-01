
// game_state.cpp
#include "game_state.h"
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <cmath>

GameState::GameState(int width, int height) : width(width), height(height), game_over(false), score(0) {
    srand(time(nullptr));
    reset();
}

GameState::~GameState() {}

void GameState::reset() {
    snake.clear();
    // Start with a snake of length 3 in the middle of the board
    int start_x = width / 2;
    int start_y = height / 2;
    snake.push_back({start_x, start_y});
    snake.push_back({start_x - 1, start_y});
    snake.push_back({start_x - 2, start_y});
    
    current_direction = Direction::RIGHT;
    game_over = false;
    score = 0;
    
    spawn_food();
}

bool GameState::update(Direction new_direction) {
    // Don't allow 180-degree turns
    if (are_opposite_directions(current_direction, new_direction)) {
        new_direction = current_direction;
    }
    
    current_direction = new_direction;
    
    // Calculate new head position
    std::pair<int, int> head = snake.front();
    std::pair<int, int> new_head = head;
    
    switch (current_direction) {
        case Direction::UP:
            new_head.second--;
            break;
        case Direction::RIGHT:
            new_head.first++;
            break;
        case Direction::DOWN:
            new_head.second++;
            break;
        case Direction::LEFT:
            new_head.first--;
            break;
    }
    
    // Check if game is over (collision with wall or self)
    if (new_head.first < 0 || new_head.first >= width ||
        new_head.second < 0 || new_head.second >= height ||
        check_collision(new_head.first, new_head.second)) {
        game_over = true;
        return false;
    }
    
    // Add new head
    snake.insert(snake.begin(), new_head);
    
    // Check if food is eaten
    if (new_head.first == food.first && new_head.second == food.second) {
        score++;
        spawn_food();
    } else {
        // Remove tail if no food eaten
        snake.pop_back();
    }
    
    return true;
}

void GameState::spawn_food() {
    int food_x, food_y;
    do {
        food_x = rand() % width;
        food_y = rand() % height;
    } while (check_collision(food_x, food_y));
    
    food = {food_x, food_y};
}

bool GameState::check_collision(int x, int y) const {
    for (const auto& segment : snake) {
        if (segment.first == x && segment.second == y) {
            return true;
        }
    }
    return false;
}

std::vector<double> GameState::get_vision_data() const {
    return get_state_representation();
}

std::vector<double> GameState::get_state_representation() const {
    std::vector<double> state;
    
    // Current direction (one-hot encoding)
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
    
    std::pair<int, int> head = snake.front();
    
    for (int i = 0; i < 8; i++) {
        double wall_distance = 1.0;
        double food_distance = 0.0;
        double self_distance = 0.0;
        bool food_found = false;
        bool self_found = false;
        
        int x = head.first;
        int y = head.second;
        int steps = 0;
        
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
            if (!food_found && x == food.first && y == food.second) {
                food_distance = 1.0 / steps;
                food_found = true;
            }
            
            // Check if self found
            if (!self_found) {
                for (size_t j = 1; j < snake.size(); j++) {
                    if (x == snake[j].first && y == snake[j].second) {
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
