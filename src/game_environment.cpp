
// game_environment.cpp
#include "game_environment.h"

GameEnvironment::GameEnvironment(int width, int height, int max_steps_without_food)
    : width(width), height(height), max_steps_without_food(max_steps_without_food) {
}

double GameEnvironment::run_game(const SnakeAgent& agent, int num_games) {
    double total_score = 0.0;
    
    for (int i = 0; i < num_games; i++) {
        Game game(width, height);
        game.set_max_steps_without_food(max_steps_without_food);
        
        while (!game.is_game_over()) {
            std::vector<double> state = game.get_state_for_ai();
            Direction move = agent.get_move(state);
            game.update(move);
        }
        
        total_score += game.get_score();
    }
    
    return total_score / num_games;
}

double GameEnvironment::calculate_fitness(const SnakeAgent& agent, int num_games) {
    double avg_score = run_game(agent, num_games);
    
    // Fitness function: score^2 to reward higher scores more
    return avg_score * avg_score;
}
