
// snake.cpp
#include "snake.h"

Snake::Snake(int start_x, int start_y, Direction initial_direction) 
    : current_direction(initial_direction) {
    // Initialize snake with 3 segments
    body.push_back({start_x, start_y});  // Head
    
    // Add segments based on initial direction
    switch (initial_direction) {
        case Direction::RIGHT:
            body.push_back({start_x - 1, start_y});
            body.push_back({start_x - 2, start_y});
            break;
        case Direction::LEFT:
            body.push_back({start_x + 1, start_y});
            body.push_back({start_x + 2, start_y});
            break;
        case Direction::DOWN:
            body.push_back({start_x, start_y - 1});
            body.push_back({start_x, start_y - 2});
            break;
        case Direction::UP:
            body.push_back({start_x, start_y + 1});
            body.push_back({start_x, start_y + 2});
            break;
    }
}

void Snake::move(Direction dir, bool grow) {
    // Don't allow 180-degree turns
    if (are_opposite_directions(current_direction, dir)) {
        dir = current_direction;
    }
    
    current_direction = dir;
    
    // Calculate new head position
    std::pair<int, int> head = body.front();
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
    
    // Add new head
    body.insert(body.begin(), new_head);
    
    // Remove tail if not growing
    if (!grow && !body.empty()) {
        body.pop_back();
    }
}

bool Snake::check_collision_with_self() const {
    if (body.size() <= 1) return false;
    
    std::pair<int, int> head = body.front();
    
    // Check if head collides with any other segment
    for (std::size_t i = 1; i < body.size(); i++) {
        if (head.first == body[i].first && head.second == body[i].second) {
            return true;
        }
    }
    
    return false;
}

bool Snake::check_collision_with_point(int x, int y) const {
    for (const auto& segment : body) {
        if (segment.first == x && segment.second == y) {
            return true;
        }
    }
    return false;
}
