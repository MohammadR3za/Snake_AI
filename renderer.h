#ifndef RENDERER_H
#define RENDERER_H

#include <SDL2/SDL.h>
#include "snake.h"

class Renderer {
public:
    Renderer(int window_width, int window_height, int grid_width, int grid_height);
    ~Renderer();
    
    void render_game(const Snake& snake);
    void delay(int ms) const;
    bool should_quit() const;
    
private:
    void render_grid();
    void render_snake(const Snake& snake);
    void render_food(const Point& food);
    void render_score(int score);
    
    SDL_Window* window;
    SDL_Renderer* renderer;
    
    int window_width;
    int window_height;
    int grid_width;
    int grid_height;
    
    int cell_width;
    int cell_height;
    
    mutable bool quit_requested;
};

#endif // RENDERER_H
