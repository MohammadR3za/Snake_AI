
// direction.h
#ifndef DIRECTION_H
#define DIRECTION_H

enum class Direction {
    UP,
    RIGHT,
    DOWN,
    LEFT
};

// Utility functions for direction conversion
inline int direction_to_int(Direction dir) {
    switch (dir) {
        case Direction::UP: return 0;
        case Direction::RIGHT: return 1;
        case Direction::DOWN: return 2;
        case Direction::LEFT: return 3;
        default: return -1;
    }
}

inline Direction int_to_direction(int dir) {
    switch (dir) {
        case 0: return Direction::UP;
        case 1: return Direction::RIGHT;
        case 2: return Direction::DOWN;
        case 3: return Direction::LEFT;
        default: return Direction::RIGHT; // Default direction
    }
}

// Check if two directions are opposite
inline bool are_opposite_directions(Direction dir1, Direction dir2) {
    return (dir1 == Direction::UP && dir2 == Direction::DOWN) ||
           (dir1 == Direction::DOWN && dir2 == Direction::UP) ||
           (dir1 == Direction::LEFT && dir2 == Direction::RIGHT) ||
           (dir1 == Direction::RIGHT && dir2 == Direction::LEFT);
}

#endif // DIRECTION_H
