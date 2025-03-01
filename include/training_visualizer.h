
// training_visualizer.h
#ifndef TRAINING_VISUALIZER_H
#define TRAINING_VISUALIZER_H

#include "snake_agent.h"
#include "game.h"
#include <ncurses.h>

class TrainingVisualizer {
public:
    TrainingVisualizer(int width = 20, int height = 20);
    ~TrainingVisualizer();
    
    void visualize_agent(const SnakeAgent& agent, int num_games = 1, int delay_ms = 100);
    void draw_game(const Game& game);
    void draw_population_game(const Game& game); // New method for population visualization
    
private:
    int width;
    int height;
    WINDOW* window;
    
    void initialize_ncurses();
    void cleanup_ncurses();
};

#endif // TRAINING_VISUALIZER_H
