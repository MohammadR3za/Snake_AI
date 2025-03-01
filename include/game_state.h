
// game_state.h
#ifndef GAME_STATE_H
#define GAME_STATE_H

#include <vector>
#include <utility>
#include "direction.h"

class GameState {
public:
    GameState(int width = 20, int height = 20);
    ~GameState();
    
    void reset();
    bool update(Direction new_direction);
    
    // Vision data for AI
    std::vector<double> get_vision_data() const;
    std::vector<double> get_state_representation() const;
    
    // Getters
    const std::vector<std::pair<int, int>>& get_snake() const { return snake; }
    std::pair<int, int> get_food() const { return food; }
    Direction get_current_direction() const { return current_direction; }
    int get_width() const { return width; }
    int get_height() const { return height; }
    int get_score() const { return score; }
    bool is_game_over() const { return game_over; }
    
private:
    int width;
    int height;
    std::vector<std::pair<int, int>> snake;
    Direction current_direction;
    std::pair<int, int> food;
    bool game_over;
    int score;
    
    void spawn_food();
    bool check_collision(int x, int y) const;
};

#endif // GAME_STATE_H
