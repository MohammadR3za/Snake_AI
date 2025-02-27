#include "snake.h"
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <iomanip>
#include <sstream>

Snake::Snake(unsigned int width, unsigned int height, GameMode mode)
    : width(width),
      height(height),
      current_direction(Direction::RIGHT),
      score(0),
      highest_score(0),
      game_over(false),
      steps_since_last_food(0),
      max_steps_without_food(100),
      game_mode(mode),
      visualize(true),
      animation_delay_ms(100),
      exploration_rate(1.0),
      exploration_min(0.01),
      exploration_decay(0.995),
      replay_buffer_size(10000),
      min_replay_size(1000),
      batch_size(32),
      discount_factor(0.99),
      target_update_frequency(100),
      current_iteration(0)
{
    // Seed random number generator
    std::random_device rd;
    rng.seed(rd());
    
    // Initialize neural network for DQN
    // Input: game state representation
    // Output: Q-values for each possible action (UP, RIGHT, DOWN, LEFT)
    std::vector<int> layer_sizes = {11, 64, 32, 4};
    nn = NeuralNetwork(layer_sizes, 0.001, NeuralNetwork::ActivationFunction::RELU, NeuralNetwork::ActivationFunction::LINEAR);
    target_nn = NeuralNetwork(layer_sizes, 0.001, NeuralNetwork::ActivationFunction::RELU, NeuralNetwork::ActivationFunction::LINEAR);
    target_nn.copy_weights_from(nn);
    
    reset();
}

void Snake::reset() {
    // Clear snake segments
    segments.clear();
    
    // Create initial snake (3 segments at the center of the board)
    int center_x = width / 2;
    int center_y = height / 2;
    
    segments.push_back(Point(center_x, center_y));
    segments.push_back(Point(center_x - 1, center_y));
    segments.push_back(Point(center_x - 2, center_y));
    
    // Reset game state
    current_direction = Direction::RIGHT;
    score = 0;
    game_over = false;
    steps_since_last_food = 0;
    
    // Create initial food
    create_food();
}

void Snake::create_food() {
    // Create a distribution for random positions
    std::uniform_int_distribution<int> x_dist(0, width - 1);
    std::uniform_int_distribution<int> y_dist(0, height - 1);
    
    // Keep generating positions until we find one that's not occupied by the snake
    bool valid_position = false;
    while (!valid_position) {
        food = Point(x_dist(rng), y_dist(rng));
        valid_position = true;
        
        // Check if the food is on any snake segment
        for (const auto& segment : segments) {
            if (food == segment) {
                valid_position = false;
                break;
            }
        }
    }
}

Point Snake::direction_to_delta(Direction dir) const {
    switch (dir) {
        case Direction::UP:    return Point(0, -1);
        case Direction::RIGHT: return Point(1, 0);
        case Direction::DOWN:  return Point(0, 1);
        case Direction::LEFT:  return Point(-1, 0);
        default:               return Point(0, 0);
    }
}

bool Snake::will_collide(Direction dir) const {
    if (dir == Direction::NONE) {
        return true;
    }
    
    // Get the next position
    Point delta = direction_to_delta(dir);
    Point next_head = segments.front() + delta;
    
    // Check if the next position is out of bounds
    if (next_head.x < 0 || next_head.x >= static_cast<int>(width) ||
        next_head.y < 0 || next_head.y >= static_cast<int>(height)) {
        return true;
    }
    
    // Check if the next position collides with the snake's body
    // Skip the last segment if the snake is not eating food (it will move out of the way)
    for (size_t i = 0; i < segments.size() - 1; ++i) {
        if (next_head == segments[i]) {
            return true;
        }
    }
    
    return false;
}

std::vector<Direction> Snake::get_valid_directions() const {
    std::vector<Direction> valid_dirs;
    
    // Check each direction
    for (int i = 0; i < 4; ++i) {
        Direction dir = static_cast<Direction>(i);
        
        // Don't allow 180-degree turns
        if ((dir == Direction::UP && current_direction == Direction::DOWN) ||
            (dir == Direction::DOWN && current_direction == Direction::UP) ||
            (dir == Direction::LEFT && current_direction == Direction::RIGHT) ||
            (dir == Direction::RIGHT && current_direction == Direction::LEFT)) {
            continue;
        }
        
        if (!will_collide(dir)) {
            valid_dirs.push_back(dir);
        }
    }
    
    return valid_dirs;
}

bool Snake::move(Direction dir) {
    if (game_over) {
        return false;
    }
    
    // If NONE is passed, continue in the current direction
    if (dir == Direction::NONE) {
        dir = current_direction;
    }
    
    // Don't allow 180-degree turns
    if ((dir == Direction::UP && current_direction == Direction::DOWN) ||
        (dir == Direction::DOWN && current_direction == Direction::UP) ||
        (dir == Direction::LEFT && current_direction == Direction::RIGHT) ||
        (dir == Direction::RIGHT && current_direction == Direction::LEFT)) {
        dir = current_direction;
    }
    
    // Update current direction
    current_direction = dir;
    
    // Calculate the next head position
    Point delta = direction_to_delta(dir);
    Point next_head = segments.front() + delta;
    
    // Check for collisions with walls
    if (next_head.x < 0 || next_head.x >= static_cast<int>(width) ||
        next_head.y < 0 || next_head.y >= static_cast<int>(height)) {
        game_over = true;
        return false;
    }
    
    // Check for collisions with self
    for (size_t i = 0; i < segments.size() - 1; ++i) {
        if (next_head == segments[i]) {
            game_over = true;
            return false;
        }
    }
    
    // Add the new head
    segments.push_front(next_head);
    
    // Check if the snake ate food
    bool ate_food = (next_head == food);
    if (ate_food) {
        // Increase score
        score++;
        if (score > highest_score) {
            highest_score = score;
        }
        
        // Create new food
        create_food();
        
        // Reset steps counter
        steps_since_last_food = 0;
    } else {
        // Remove the tail segment
        segments.pop_back();
        
        // Increment steps counter
        steps_since_last_food++;
        
        // Check if the snake has been wandering too long without food
        if (steps_since_last_food >= max_steps_without_food) {
            game_over = true;
            return false;
        }
    }
    
    return true;
}

double Snake::calculate_distance(const Point& p1, const Point& p2) const {
    // Manhattan distance
    return std::abs(p1.x - p2.x) + std::abs(p1.y - p2.y);
}

std::vector<double> Snake::get_state() const {
    std::vector<double> state;
    
    // Head position (normalized)
    state.push_back(static_cast<double>(segments.front().x) / width);
    state.push_back(static_cast<double>(segments.front().y) / height);
    
    // Food position (normalized)
    state.push_back(static_cast<double>(food.x) / width);
    state.push_back(static_cast<double>(food.y) / height);
    
    // Direction one-hot encoding
    for (int i = 0; i < 4; ++i) {
        state.push_back(current_direction == static_cast<Direction>(i) ? 1.0 : 0.0);
    }
    
    // Danger detection (binary values: 1 if danger in that direction)
    Point head = segments.front();
    
    // Check danger straight ahead
    Direction straight = current_direction;
    Point straight_pos = head + direction_to_delta(straight);
    bool danger_straight = (straight_pos.x < 0 || straight_pos.x >= static_cast<int>(width) ||
                           straight_pos.y < 0 || straight_pos.y >= static_cast<int>(height));
    
    if (!danger_straight) {
        for (size_t i = 1; i < segments.size(); ++i) {
            if (straight_pos == segments[i]) {
                danger_straight = true;
                break;
            }
        }
    }
    
    // Check danger to the right
    Direction right;
    switch (current_direction) {
        case Direction::UP:    right = Direction::RIGHT; break;
        case Direction::RIGHT: right = Direction::DOWN; break;
        case Direction::DOWN:  right = Direction::LEFT; break;
        case Direction::LEFT:  right = Direction::UP; break;
        default:               right = Direction::NONE;
    }
    
    Point right_pos = head + direction_to_delta(right);
    bool danger_right = (right_pos.x < 0 || right_pos.x >= static_cast<int>(width) ||
                        right_pos.y < 0 || right_pos.y >= static_cast<int>(height));
    
    if (!danger_right) {
        for (size_t i = 1; i < segments.size(); ++i) {
            if (right_pos == segments[i]) {
                danger_right = true;
                break;
            }
        }
    }
    
    // Check danger to the left
    Direction left;
    switch (current_direction) {
        case Direction::UP:    left = Direction::LEFT; break;
        case Direction::RIGHT: left = Direction::UP; break;
        case Direction::DOWN:  left = Direction::RIGHT; break;
        case Direction::LEFT:  left = Direction::DOWN; break;
        default:               left = Direction::NONE;
    }
    
    Point left_pos = head + direction_to_delta(left);
    bool danger_left = (left_pos.x < 0 || left_pos.x >= static_cast<int>(width) ||
                       left_pos.y < 0 || left_pos.y >= static_cast<int>(height));
    
    if (!danger_left) {
        for (size_t i = 1; i < segments.size(); ++i) {
            if (left_pos == segments[i]) {
                danger_left = true;
                break;
            }
        }
    }
    
    state.push_back(danger_straight ? 1.0 : 0.0);
    state.push_back(danger_right ? 1.0 : 0.0);
    state.push_back(danger_left ? 1.0 : 0.0);
    
    return state;
}

Direction Snake::choose_action(const std::vector<double>& state, bool explore) {
    // Exploration: choose a random valid direction
    if (explore && std::uniform_real_distribution<>(0, 1)(rng) < exploration_rate) {
        std::vector<Direction> valid_dirs = get_valid_directions();
        if (valid_dirs.empty()) {
            return Direction::NONE;  // No valid moves
        }
        std::uniform_int_distribution<size_t> dist(0, valid_dirs.size() - 1);
        return valid_dirs[dist(rng)];
    }
    
    // Exploitation: choose the best action according to the neural network
    std::vector<double> q_values = nn.predict(state);
    
    // Find the best valid action
    Direction best_action = Direction::NONE;
    double best_value = -std::numeric_limits<double>::infinity();
    
    for (int i = 0; i < 4; ++i) {
        Direction dir = static_cast<Direction>(i);
        
        // Skip invalid directions (180-degree turns)
        if ((dir == Direction::UP && current_direction == Direction::DOWN) ||
            (dir == Direction::DOWN && current_direction == Direction::UP) ||
            (dir == Direction::LEFT && current_direction == Direction::RIGHT) ||
            (dir == Direction::RIGHT && current_direction == Direction::LEFT)) {
            continue;
        }
        
        // Skip directions that would lead to immediate collision
        if (will_collide(dir)) {
            continue;
        }
        
        if (q_values[i] > best_value) {
            best_value = q_values[i];
            best_action = dir;
        }
    }
    
    // If no valid action was found, try to find any non-colliding direction
    if (best_action == Direction::NONE) {
        std::vector<Direction> valid_dirs = get_valid_directions();
        if (!valid_dirs.empty()) {
            return valid_dirs[0];
        }
    }
    
    return best_action;
}

void Snake::add_to_replay_memory(Experience&& exp) {
    replay_buffer.push_back(std::move(exp));
    if (replay_buffer.size() > replay_buffer_size) {
        replay_buffer.pop_front();
    }
}

void Snake::train_from_replay_memory() {
    if (replay_buffer.size() < min_replay_size) {
        return;  // Not enough experiences yet
    }
    
    // Sample a batch of experiences
    std::vector<Experience> batch;
    std::vector<int> indices;
    
    // Create a list of indices and shuffle it
    for (size_t i = 0; i < replay_buffer.size(); ++i) {
        indices.push_back(i);
    }
    std::shuffle(indices.begin(), indices.end(), rng);
    
    // Take the first batch_size indices
    for (size_t i = 0; i < batch_size && i < indices.size(); ++i) {
        batch.push_back(replay_buffer[indices[i]]);
    }
    
    // Prepare training data
    std::vector<std::vector<double>> states;
    std::vector<std::vector<double>> targets;
    
    for (const auto& exp : batch) {
        // Get the current Q-values for the state
        std::vector<double> q_values = nn.predict(exp.state);
        
        // Calculate the target Q-value for the action taken
        double target_q;
        
        if (exp.done) {
            // Terminal state
            target_q = exp.reward;
        } else {
            // Non-terminal state: Q(s,a) = r + γ * max(Q(s',a'))
            std::vector<double> next_q_values = target_nn.predict(exp.next_state);
            double max_next_q = *std::max_element(next_q_values.begin(), next_q_values.end());
            target_q = exp.reward + discount_factor * max_next_q;
        }
        
        // Update the Q-value for the action that was taken
        q_values[exp.action] = target_q;
        
        // Add to training data
        states.push_back(exp.state);
        targets.push_back(q_values);
    }
    
    // Train the network on this batch
    nn.train_batch(states, targets);
    
    // Periodically update the target network
    if (++current_iteration % target_update_frequency == 0) {
        target_nn.copy_weights_from(nn);
    }
    
    // Decay exploration rate
    exploration_rate = std::max(exploration_min, exploration_rate * exploration_decay);
}

Direction Snake::get_user_input() const {
    // This is a placeholder. In a real implementation, you would get input from the keyboard.
    // For example, using ncurses, SDL, or another input library.
    
    // For demonstration purposes, we'll just return the current direction
    return current_direction;
}

void Snake::render() const {
    // Clear screen
    std::cout << "\033[2J\033[1;1H";
    
    // Print top border
    std::cout << "+";
    for (unsigned int x = 0; x < width; ++x) {
        std::cout << "-";
    }
    std::cout << "+\n";
    
    // Print game board
    for (unsigned int y = 0; y < height; ++y) {
        std::cout << "|";
        for (unsigned int x = 0; x < width; ++x) {
            Point p(x, y);
            
            if (p == segments.front()) {
                std::cout << "O";  // Snake head
            } else if (p == food) {
                std::cout << "F";  // Food
            } else {
                bool is_segment = false;
                for (size_t i = 1; i < segments.size(); ++i) {
                    if (p == segments[i]) {
                        std::cout << "o";  // Snake body
                        is_segment = true;
                        break;
                    }
                }
                if (!is_segment) {
                    std::cout << " ";  // Empty space
                }
            }
        }
        std::cout << "|\n";
    }
    
    // Print bottom border
    std::cout << "+";
    for (unsigned int x = 0; x < width; ++x) {
        std::cout << "-";
    }
    std::cout << "+\n";
    
    // Print score
    std::cout << "Score: " << score << " | Highest: " << highest_score << std::endl;
    
    if (game_mode == GameMode::TRAINING) {
        std::cout << "Exploration rate: " << exploration_rate << std::endl;
        std::cout << "Replay buffer: " << replay_buffer.size() << "/" << replay_buffer_size << std::endl;
    }
}

void Snake::play() {
    while (!game_over) {
        // Render the game
        if (visualize) {
            render();
            std::this_thread::sleep_for(std::chrono::milliseconds(animation_delay_ms));
        }
        
        Direction next_dir;
        
        if (game_mode == GameMode::MANUAL) {
            // Get user input
            next_dir = get_user_input();
        } else {
            // Use AI to choose the next move
            next_dir = choose_action(get_state(), false);
        }
        
        // Move the snake
        move(next_dir);
    }
    
    // Game over
    if (visualize) {
        render();
        std::cout << "Game Over!" << std::endl;
    }
}

void Snake::train(int episodes, int max_steps) {
    for (int episode = 0; episode < episodes; ++episode) {
        reset();
        
        double total_reward = 0.0;
        double prev_distance = calculate_distance(segments.front(), food);
        
        for (int step = 0; step < max_steps && !game_over; ++step) {
            // Get current state
            std::vector<double> state = get_state();
            
            // Choose action
            Direction action = choose_action(state, true);
            
            // Take action
            bool moved = move(action);
            
            // Calculate reward
            double reward = Reward::SURVIVAL;  // Small reward for surviving
            
            if (!moved || game_over) {
                reward = Reward::DEATH;  // Penalty for dying
            } else {
                // Check if the snake ate food
                if (segments.front() == food) {
                    reward = Reward::EAT_FOOD;  // Reward for eating food
                } else {
                    // Reward based on distance to food
                    double current_distance = calculate_distance(segments.front(), food);
                    if (current_distance < prev_distance) {
                        reward += Reward::MOVE_CLOSER_TO_FOOD;  // Reward for moving closer to food
                    } else if (current_distance > prev_distance) {
                        reward += Reward::MOVE_AWAY_FROM_FOOD;  // Penalty for moving away from food
                    }
                    prev_distance = current_distance;
                }
            }
            
            total_reward += reward;
            
            // Get next state
            std::vector<double> next_state = get_state();
            
            // Store experience in replay memory
            add_to_replay_memory({
                state,
                static_cast<int>(action),
                reward,
                next_state,
                game_over
            });
            
            // Train from replay memory
            train_from_replay_memory();
            
            // Visualize if enabled
            if (visualize) {
                render();
                std::this_thread::sleep_for(std::chrono::milliseconds(animation_delay_ms));
            }
        }
        
        // Print episode statistics
        if ((episode + 1) % 10 == 0 || episode == 0) {
            std::cout << "Episode " << (episode + 1) << "/" << episodes
                      << " | Score: " << score
                      << " | Total Reward: " << total_reward
                      << " | Exploration: " << exploration_rate
                      << std::endl;
        }
    }
}

bool Snake::save_model(const std::string& filename) {
    return nn.save(filename);
}

bool Snake::load_model(const std::string& filename) {
    bool success = nn.load(filename);
    if (success) {
        target_nn.copy_weights_from(nn);
    }
    return success;
}

bool Snake::load_config(const std::string& filename) {
    std::ifstream file(filename);
    if (!file) {
        std::cerr << "Error: Could not open config file: " << filename << std::endl;
        return false;
    }
    
    std::string line;
    std::string current_section;
    
    while (std::getline(file, line)) {
        // Remove leading/trailing whitespace
        line.erase(0, line.find_first_not_of(" \t"));
        line.erase(line.find_last_not_of(" \t") + 1);
        
        // Skip empty lines and comments
        if (line.empty() || line[0] == ';' || line[0] == '#') {
            continue;
        }
        
        // Check for section header
        if (line[0] == '[' && line.back() == ']') {
            current_section = line.substr(1, line.size() - 2);
            continue;
        }
        
        // Parse key-value pair
        size_t delimiter_pos = line.find('=');
        if (delimiter_pos != std::string::npos) {
            std::string key = line.substr(0, delimiter_pos);
            std::string value = line.substr(delimiter_pos + 1);
            
            // Remove whitespace
            key.erase(0, key.find_first_not_of(" \t"));
            key.erase(key.find_last_not_of(" \t") + 1);
            value.erase(0, value.find_first_not_of(" \t"));
            value.erase(value.find_last_not_of(" \t") + 1);
            
            // Parse configuration values based on section and key
            if (current_section == "Game" || current_section.empty()) {
                if (key == "width") {
                    width = std::stoi(value);
                } else if (key == "height") {
                    height = std::stoi(value);
                } else if (key == "visualize") {
                    visualize = (value == "true" || value == "1");
                } else if (key == "animation_delay_ms") {
                    animation_delay_ms = std::stoi(value);
                } else if (key == "max_steps_without_food") {
                    max_steps_without_food = std::stoi(value);
                }
            } else if (current_section == "Training") {
                if (key == "exploration_rate") {
                    exploration_rate = std::stod(value);
                } else if (key == "exploration_min") {
                    exploration_min = std::stod(value);
                } else if (key == "exploration_decay") {
                    exploration_decay = std::stod(value);
                } else if (key == "replay_buffer_size") {
                    replay_buffer_size = std::stoul(value);
                } else if (key == "min_replay_size") {
                    min_replay_size = std::stoul(value);
                } else if (key == "batch_size") {
                    batch_size = std::stoul(value);
                } else if (key == "discount_factor") {
                    discount_factor = std::stod(value);
                } else if (key == "target_update_frequency") {
                    target_update_frequency = std::stoi(value);
                }
            }
        }
    }
    
    return true;
}
