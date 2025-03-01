
// play_mode.h
#ifndef PLAY_MODE_H
#define PLAY_MODE_H

#include "game.h"
#include "snake_agent.h"
#include <ncurses.h>
#include <string>

class PlayMode {
public:
    PlayMode(int width = 20, int height = 20);
    ~PlayMode();
    
    void human_play();
    void ai_play(const std::string& model_file);
    
private:
    int width;
    int height;
    WINDOW* window;
    
    void initialize_ncurses();
    void cleanup_ncurses();
    void draw_game(const Game& game);
    Direction get_human_input(Direction current_dir);
};

#endif // PLAY_MODE_H
