#include "renderer.h"
#include <SDL2/SDL_ttf.h>
#include <string>
#include <iostream>

Renderer::Renderer(int window_width, int window_height, int grid_width, int grid_height)
    : window_width(window_width),
      window_height(window_height),
      grid_width(grid_width),
      grid_height(grid_height),
      quit_requested(false) {
    
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        return;
    }
    
    // Initialize SDL_ttf for text rendering
    if (TTF_Init() < 0) {
        std::cerr << "SDL_ttf could not initialize! TTF_Error: " << TTF_GetError() << std::endl;
    }
    
    // Create window
    window = SDL_CreateWindow("Snake AI", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                             window_width, window_height, SDL_WINDOW_SHOWN);
    if (!window) {
        std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        return;
    }
    
    // Create renderer
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        std::cerr << "Renderer could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        return;
    }
    
    // Calculate cell dimensions
    cell_width = window_width / grid_width;
    cell_height = window_height / grid_height;
}

Renderer::~Renderer() {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
}

void Renderer::render_game(const Snake& snake) {
    // Handle events
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            quit_requested = true;
        }
    }
    
    // Clear screen
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    
    // Render grid
    render_grid();
    
    // Render food
    render_food(snake.get_food());
    
    // Render snake
    render_snake(snake);
    
    // Render score
    render_score(snake.get_score());
    
    // Update screen
    SDL_RenderPresent(renderer);
}

void Renderer::delay(int ms) const {
    SDL_Delay(ms);
}

bool Renderer::should_quit() const {
    return quit_requested;
}

void Renderer::render_grid() {
    SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
    
    // Draw vertical lines
    for (int x = 0; x <= grid_width; ++x) {
        SDL_RenderDrawLine(renderer, x * cell_width, 0, x * cell_width, window_height);
    }
    
    // Draw horizontal lines
    for (int y = 0; y <= grid_height; ++y) {
        SDL_RenderDrawLine(renderer, 0, y * cell_height, window_width, y * cell_height);
    }
}

void Renderer::render_snake(const Snake& snake) {
    const auto& body = snake.get_body();
    
    // Render head
    if (!body.empty()) {
        SDL_Rect head_rect{
            body.front().x * cell_width,
            body.front().y * cell_height,
            cell_width,
            cell_height
        };
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);  // Green
        SDL_RenderFillRect(renderer, &head_rect);
    }
    
    // Render body segments
    SDL_SetRenderDrawColor(renderer, 0, 200, 0, 255);  // Darker green
    for (size_t i = 1; i < body.size(); ++i) {
        SDL_Rect segment_rect{
            body[i].x * cell_width,
            body[i].y * cell_height,
            cell_width,
            cell_height
        };
        SDL_RenderFillRect(renderer, &segment_rect);
    }
}

void Renderer::render_food(const Point& food) {
    SDL_Rect food_rect{
        food.x * cell_width,
        food.y * cell_height,
        cell_width,
        cell_height
    };
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);  // Red
    SDL_RenderFillRect(renderer, &food_rect);
}

void Renderer::render_score(int score) {
    // Try to load a font
    TTF_Font* font = TTF_OpenFont("arial.ttf", 24);
    if (!font) {
        // If we can't load a font, just return
        return;
    }
    
    // Create text surface
    SDL_Color text_color = {255, 255, 255, 255};  // White
    std::string score_text = "Score: " + std::to_string(score);
    SDL_Surface* text_surface = TTF_RenderText_Solid(font, score_text.c_str(), text_color);
    
    if (text_surface) {
        // Create texture from surface
        SDL_Texture* text_texture = SDL_CreateTextureFromSurface(renderer, text_surface);
        
        if (text_texture) {
            // Render text
            SDL_Rect text_rect{10, 10, text_surface->w, text_surface->h};
            SDL_RenderCopy(renderer, text_texture, nullptr, &text_rect);
            SDL_DestroyTexture(text_texture);
        }
        
        SDL_FreeSurface(text_surface);
    }
    
    TTF_CloseFont(font);
}
