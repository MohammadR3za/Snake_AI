
// game_environment.h
#ifndef GAME_ENVIRONMENT_H
#define GAME_ENVIRONMENT_H

#include "game.h"
#include "snake_agent.h"

class GameEnvironment {
public:
    GameEnvironment(int width = 20, int height = 20, int max_steps_without_food = 100);
    
    double run_game(const SnakeAgent& agent, int num_games = 1);
    double calculate_fitness(const SnakeAgent& agent, int num_games = 1);
    
private:
    int width;
    int height;
    int max_steps_without_food;
};

#endif // GAME_ENVIRONMENT_H
