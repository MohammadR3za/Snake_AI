#include "snake.h"
#include <cmath>
#include <algorithm>

Snake::Snake(int grid_width, int grid_height)
    : grid_width(grid_width),
      grid_height(grid_height),
      max_steps_without_food(grid_width * grid_height),
      rng(std::random_device{}()) {
    
    // Initialize snake at the center of the grid
    snake_body.emplace_back(grid_width / 2, grid_height / 2);
    
    // Initialize visited cells tracking
    visited_cells.resize(grid_height, std::vector<bool>(grid_width, false));
    visited_cells[grid_height / 2][grid_width / 2] = true;
    cells_visited = 1;
    
    // Place initial food
    place_food();
}

void Snake::update() {
    // Get direction from AI if available
    if (network) {
        direction = get_ai_direction();
    }
    
    // Move snake head in the current direction
    Point& head = snake_body.front();
    Point new_head = head;
    
    switch (direction) {
        case Direction::UP:
            new_head.y -= 1;
            break;
        case Direction::RIGHT:
            new_head.x += 1;
            break;
        case Direction::DOWN:
            new_head.y += 1;
            break;
        case Direction::LEFT:
            new_head.x -= 1;
            break;
    }
    
    // Handle wrap-around
    if (new_head.x < 0) new_head.x = grid_width - 1;
    if (new_head.x >= grid_width) new_head.x = 0;
    if (new_head.y < 0) new_head.y = grid_height - 1;
    if (new_head.y >= grid_height) new_head.y = 0;
    
    // Insert new head
    snake_body.insert(snake_body.begin(), new_head);
    
    // Track visited cells for exploration metric
    if (!visited_cells[new_head.y][new_head.x]) {
        visited_cells[new_head.y][new_head.x] = true;
        cells_visited++;
    }
    
    // Check if food is eaten
    if (new_head.x == food.x && new_head.y == food.y) {
        score++;
        steps_since_last_food = 0;
        place_food();
        // Don't remove tail (snake grows)
    } else {
        // Remove tail if no food eaten
        snake_body.pop_back();
        steps_since_last_food++;
    }
    
    steps_taken++;
    
    // Update efficiency factor (score per step)
    if (steps_taken > 0) {
        efficiency_factor = static_cast<double>(score) / steps_taken;
    }
    
    // Update exploration factor (percentage of grid explored)
    exploration_factor = static_cast<double>(cells_visited) / (grid_width * grid_height);
}

bool Snake::is_game_over() const {
    // Game over if snake collides with itself or exceeds max steps without food
    return check_collision() || steps_since_last_food >= max_steps_without_food;
}

double Snake::calculate_fitness() const {
    // Base fitness based on score with exponential scaling
    double fitness = std::pow(4.0, static_cast<double>(score));
    
    // Add bonus for steps taken (with diminishing returns to prevent stalling)
    fitness += std::sqrt(static_cast<double>(steps_taken)) * 0.5;
    
    // Add bonus for efficiency (score relative to steps)
    fitness += efficiency_factor * 100.0;
    
    // Add bonus for exploration
    fitness += exploration_factor * 50.0;
    
    // Add bonus for getting close to food at the end
    Point head = snake_body.front();
    double distance_to_food = std::sqrt(
        std::pow(head.x - food.x, 2) + 
        std::pow(head.y - food.y, 2)
    );
    
    // Inverse distance bonus (smaller distance = higher bonus)
    if (distance_to_food > 0) {
        fitness += 10.0 / distance_to_food;
    }
    
    // Penalty for dying early
    if (steps_taken < 10) {
        fitness *= 0.5;
    }
    
    return fitness;
}

void Snake::set_direction(Direction dir) {
    // Prevent 180-degree turns (can't go directly backwards)
    if (direction == Direction::UP && dir == Direction::DOWN) return;
    if (direction == Direction::DOWN && dir == Direction::UP) return;
    if (direction == Direction::LEFT && dir == Direction::RIGHT) return;
    if (direction == Direction::RIGHT && dir == Direction::LEFT) return;
    
    direction = dir;
}

void Snake::set_network(std::unique_ptr<NeuralNetwork> net) {
    network = std::move(net);
}

void Snake::place_food() {
    std::uniform_int_distribution<int> x_dist(0, grid_width - 1);
    std::uniform_int_distribution<int> y_dist(0, grid_height - 1);
    
    // Keep generating positions until we find one that's not on the snake
    while (true) {
        Point new_food{x_dist(rng), y_dist(rng)};
        bool on_snake = false;
        
        for (const auto& segment : snake_body) {
            if (segment.x == new_food.x && segment.y == new_food.y) {
                on_snake = true;
                break;
            }
        }
        
        if (!on_snake) {
            food = new_food;
            break;
        }
    }
}

bool Snake::check_collision() const {
    if (snake_body.size() <= 1) return false;
    
    const Point& head = snake_body.front();
    
    // Check collision with snake body (skip the head)
    for (size_t i = 1; i < snake_body.size(); ++i) {
        if (head.x == snake_body[i].x && head.y == snake_body[i].y) {
            return true;
        }
    }
    
    return false;
}

Direction Snake::get_ai_direction() {
    if (!network) return direction;
    
    // Get inputs for neural network
    std::vector<double> inputs = get_network_inputs();
    
    // Get outputs from neural network
    std::vector<double> outputs = network->forward(inputs);
    
    // Find the highest output value
    int max_index = 0;
    double max_value = outputs[0];
    
    for (size_t i = 1; i < outputs.size(); ++i) {
        if (outputs[i] > max_value) {
            max_value = outputs[i];
            max_index = i;
        }
    }
    
    // Convert to direction
    Direction new_direction;
    switch (max_index) {
        case 0: new_direction = Direction::UP; break;
        case 1: new_direction = Direction::RIGHT; break;
        case 2: new_direction = Direction::DOWN; break;
        case 3: new_direction = Direction::LEFT; break;
        default: new_direction = direction; break;
    }
    
    // Prevent 180-degree turns
    if ((direction == Direction::UP && new_direction == Direction::DOWN) ||
        (direction == Direction::DOWN && new_direction == Direction::UP) ||
        (direction == Direction::LEFT && new_direction == Direction::RIGHT) ||
        (direction == Direction::RIGHT && new_direction == Direction::LEFT)) {
        return direction;
    }
    
    return new_direction;
}

std::vector<double> Snake::get_network_inputs() const {
    std::vector<double> inputs;
    const Point& head = snake_body.front();
    
    // Normalized head position
    inputs.push_back(static_cast<double>(head.x) / grid_width);
    inputs.push_back(static_cast<double>(head.y) / grid_height);
    
    // Normalized food position
    inputs.push_back(static_cast<double>(food.x) / grid_width);
    inputs.push_back(static_cast<double>(food.y) / grid_height);
    
    // Direction vector to food
    inputs.push_back((food.x - head.x) / static_cast<double>(grid_width));
    inputs.push_back((food.y - head.y) / static_cast<double>(grid_height));
    
    // Danger detection in each direction (1.0 = danger, 0.0 = safe)
    // Check UP
    bool danger_up = false;
    Point check = head;
    check.y -= 1;
    if (check.y < 0) check.y = grid_height - 1;  // Wrap around
    
    for (size_t i = 1; i < snake_body.size(); ++i) {
        if (check.x == snake_body[i].x && check.y == snake_body[i].y) {
            danger_up = true;
            break;
        }
    }
    inputs.push_back(danger_up ? 1.0 : 0.0);
    
    // Check RIGHT
    bool danger_right = false;
    check = head;
    check.x += 1;
    if (check.x >= grid_width) check.x = 0;  // Wrap around
    
    for (size_t i = 1; i < snake_body.size(); ++i) {
        if (check.x == snake_body[i].x && check.y == snake_body[i].y) {
            danger_right = true;
            break;
        }
    }
    inputs.push_back(danger_right ? 1.0 : 0.0);
    
    // Check DOWN
    bool danger_down = false;
    check = head;
    check.y += 1;
    if (check.y >= grid_height) check.y = 0;  // Wrap around
    
    for (size_t i = 1; i < snake_body.size(); ++i) {
        if (check.x == snake_body[i].x && check.y == snake_body[i].y) {
            danger_down = true;
            break;
        }
    }
    inputs.push_back(danger_down ? 1.0 : 0.0);
    
    // Check LEFT
    bool danger_left = false;
    check = head;
    check.x -= 1;
    if (check.x < 0) check.x = grid_width - 1;  // Wrap around
    
    for (size_t i = 1; i < snake_body.size(); ++i) {
        if (check.x == snake_body[i].x && check.y == snake_body[i].y) {
            danger_left = true;
            break;
        }
    }
    inputs.push_back(danger_left ? 1.0 : 0.0);
    
    // Current direction one-hot encoding
    inputs.push_back(direction == Direction::UP ? 1.0 : 0.0);
    inputs.push_back(direction == Direction::RIGHT ? 1.0 : 0.0);
    inputs.push_back(direction == Direction::DOWN ? 1.0 : 0.0);
    inputs.push_back(direction == Direction::LEFT ? 1.0 : 0.0);
    
    return inputs;
}
