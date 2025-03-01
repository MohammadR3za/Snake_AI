
// play_mode.cpp
#include "play_mode.h"
#include <thread>
#include <chrono>

PlayMode::PlayMode(int width, int height) 
    : width(width), height(height), window(nullptr) {
    initialize_ncurses();
}

PlayMode::~PlayMode() {
    cleanup_ncurses();
}

void PlayMode::initialize_ncurses() {
    // Initialize ncurses
    initscr();
    cbreak();
    noecho();
    curs_set(0);  // Hide cursor
    
    // Create window
    window = newwin(height + 4, width + 2, 1, 1);
    keypad(window, TRUE);
    timeout(0);  // Non-blocking input
    
    // Enable colors if terminal supports them
    if (has_colors()) {
        start_color();
        init_pair(1, COLOR_GREEN, COLOR_BLACK);  // Snake
        init_pair(2, COLOR_RED, COLOR_BLACK);    // Food
        init_pair(3, COLOR_WHITE, COLOR_BLACK);  // Border
    }
}

void PlayMode::cleanup_ncurses() {
    if (window) {
        delwin(window);
        window = nullptr;
    }
    endwin();
}

void PlayMode::human_play() {
    Game game(width, height);
    Direction current_dir = Direction::RIGHT;
    
    int delay_ms = 100;  // Initial delay between frames
    
    while (!game.is_game_over()) {
        // Get input
        Direction new_dir = get_human_input(current_dir);
        current_dir = new_dir;
        
        // Update game
        game.update(current_dir);
        
        // Draw game
        draw_game(game);
        
        // Delay for gameplay speed
        std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
    }
    
    // Display game over message
    mvwprintw(window, height + 3, 2, "Game Over! Score: %d", game.get_score());
    wrefresh(window);
    
    // Wait for key press before exiting
    timeout(-1);  // Blocking input
    wgetch(window);
}

void PlayMode::ai_play(const std::string& model_file) {
    // Load AI agent
    SnakeAgent agent;
    agent.load_from_file(model_file);
    
    Game game(width, height);
    game.set_max_steps_without_food(100);
    
    int delay_ms = 100;  // Delay between frames
    
    while (!game.is_game_over()) {
        // Get AI move
        std::vector<double> state = game.get_state_for_ai();
        Direction move = agent.get_move(state);
        
        // Update game
        game.update(move);
        
        // Draw game
        draw_game(game);
        
        // Check for quit key
        int key = wgetch(window);
        if (key == 'q' || key == 'Q') {
            break;
        }
        
        // Delay for visualization
        std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
    }
    
    // Display game over message
    mvwprintw(window, height + 3, 2, "Game Over! Score: %d", game.get_score());
    wrefresh(window);
    
    // Wait for key press before exiting
    timeout(-1);  // Blocking input
    wgetch(window);
}

void PlayMode::draw_game(const Game& game) {
    wclear(window);
    
    // Draw border
    if (has_colors()) {
        wattron(window, COLOR_PAIR(3));
    }
    box(window, 0, 0);
    if (has_colors()) {
        wattroff(window, COLOR_PAIR(3));
    }
    
    // Draw snake
    if (has_colors()) {
        wattron(window, COLOR_PAIR(1));
    }
    const auto& snake_body = game.get_snake().get_body();
    for (const auto& segment : snake_body) {
        mvwaddch(window, segment.second + 1, segment.first + 1, 'O');
    }
    if (has_colors()) {
        wattroff(window, COLOR_PAIR(1));
    }
    
    // Draw food
    if (has_colors()) {
        wattron(window, COLOR_PAIR(2));
    }
    const auto& food = game.get_food();
    mvwaddch(window, food.get_y() + 1, food.get_x() + 1, 'X');
    if (has_colors()) {
        wattroff(window, COLOR_PAIR(2));
    }
    
    // Display score and controls
    mvwprintw(window, height + 2, 2, "Score: %d  Steps: %d", game.get_score(), game.get_steps());
    mvwprintw(window, height + 3, 2, "Controls: Arrow Keys = Move, Q = Quit");
    
    wrefresh(window);
}

Direction PlayMode::get_human_input(Direction current_dir) {
    int key = wgetch(window);
    Direction new_dir = current_dir;
    
    switch (key) {
        case KEY_UP:
            if (current_dir != Direction::DOWN) {
                new_dir = Direction::UP;
            }
            break;
        case KEY_RIGHT:
            if (current_dir != Direction::LEFT) {
                new_dir = Direction::RIGHT;
            }
            break;
        case KEY_DOWN:
            if (current_dir != Direction::UP) {
                new_dir = Direction::DOWN;
            }
            break;
        case KEY_LEFT:
            if (current_dir != Direction::RIGHT) {
                new_dir = Direction::LEFT;
            }
            break;
    }
    
    return new_dir;
}
