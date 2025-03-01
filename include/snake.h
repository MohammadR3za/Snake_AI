
// snake.h
#ifndef SNAKE_H
#define SNAKE_H

#include <vector>
#include <utility>
#include "direction.h"

class Snake {
public:
    Snake(int start_x, int start_y, Direction initial_direction = Direction::RIGHT);
    
    void move(Direction dir, bool grow = false);
    bool check_collision_with_self() const;
    bool check_collision_with_point(int x, int y) const;
    
    // Getters
    const std::vector<std::pair<int, int>>& get_body() const { return body; }
    std::pair<int, int> get_head() const { return body.front(); }
    Direction get_direction() const { return current_direction; }
    
private:
    std::vector<std::pair<int, int>> body;
    Direction current_direction;
};

#endif // SNAKE_H
