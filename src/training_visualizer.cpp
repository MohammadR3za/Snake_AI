
// training_visualizer.cpp
#include "training_visualizer.h"
#include <thread>
#include <chrono>

TrainingVisualizer::TrainingVisualizer(int width, int height) 
    : width(width), height(height), window(nullptr) {
    initialize_ncurses();
}

TrainingVisualizer::~TrainingVisualizer() {
    cleanup_ncurses();
}

void TrainingVisualizer::initialize_ncurses() {
    initscr();
    cbreak();
    noecho();
    curs_set(0);  // Hide cursor
    timeout(0);   // Non-blocking input
    
    // Calculate window size (add 2 for borders)
    int win_height = height + 5;  // Extra space for status info
    int win_width = width + 2;
    
    // Create window centered on screen
    int start_y = (LINES - win_height) / 2;
    int start_x = (COLS - win_width) / 2;
    window = newwin(win_height, win_width, start_y, start_x);
    
    // Enable colors if terminal supports them
    if (has_colors()) {
        start_color();
        init_pair(1, COLOR_GREEN, COLOR_BLACK);   // Snake 1
        init_pair(2, COLOR_RED, COLOR_BLACK);     // Food
        init_pair(3, COLOR_WHITE, COLOR_BLACK);   // Border
        init_pair(4, COLOR_BLUE, COLOR_BLACK);    // Snake 2
        init_pair(5, COLOR_YELLOW, COLOR_BLACK);  // Snake 3
        init_pair(6, COLOR_CYAN, COLOR_BLACK);    // Snake 4
        init_pair(7, COLOR_MAGENTA, COLOR_BLACK); // Snake 5
    }
    
    keypad(window, TRUE);  // Enable special keys
    box(window, 0, 0);     // Draw border
    wrefresh(window);
}


void TrainingVisualizer::cleanup_ncurses() {
    if (window) {
        delwin(window);
        window = nullptr;
    }
    endwin();
}

void TrainingVisualizer::visualize_agent(const SnakeAgent& agent, int num_games, int delay_ms) {
    for (int game_num = 0; game_num < num_games; game_num++) {
        Game game(width, height);
        game.set_max_steps_without_food(100);
        
        while (!game.is_game_over()) {
            // Get AI move
            std::vector<double> state = game.get_state_for_ai();
            Direction move = agent.get_move(state);
            
            // Update game
            game.update(move);
            
            // Draw game
            draw_game(game);
            
            // Delay for visualization
            std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
        }
        
        // Display game over message
        mvwprintw(window, height + 3, 2, "Game %d - Score: %d", game_num + 1, game.get_score());
        wrefresh(window);
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
}

void TrainingVisualizer::draw_game(const Game& game) {
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
        mvwaddch(window, segment.second + 1, segment.first + 1, ACS_BLOCK);
    }
    if (has_colors()) {
        wattroff(window, COLOR_PAIR(1));
    }
    
    // Draw food
    if (has_colors()) {
        wattron(window, COLOR_PAIR(2));
    }
    const auto& food = game.get_food();
    mvwaddch(window, food.get_y() + 1, food.get_x() + 1, ACS_BLOCK);
    if (has_colors()) {
        wattroff(window, COLOR_PAIR(2));
    }
    
    // Display score
    mvwprintw(window, height + 2, 2, "Score: %d  Steps: %d", game.get_score(), game.get_steps());
    
    wrefresh(window);
}

void TrainingVisualizer::draw_population_game(const Game& game) {
    wclear(window);
    
    // Draw border
    if (has_colors()) {
        wattron(window, COLOR_PAIR(3));
    }
    box(window, 0, 0);
    if (has_colors()) {
        wattroff(window, COLOR_PAIR(3));
    }
    
    // Draw all snakes
    for (int i = 0; i < game.get_population_size(); i++) {
        if (!game.is_snake_alive(i)) continue;
        
        // Use different colors for different snakes if possible
        int color_pair = 1;
        if (has_colors() && i < 7) { // We have 7 distinct color pairs
            color_pair = i + 1;
            wattron(window, COLOR_PAIR(color_pair));
        }
        
        const auto& snake_body = game.get_snake(i).get_body();
        for (const auto& segment : snake_body) {
            mvwaddch(window, segment.second + 1, segment.first + 1, ACS_BLOCK);
        }
        
        if (has_colors() && i < 7) {
            wattroff(window, COLOR_PAIR(color_pair));
        }
    }
    
    // Draw food
    if (has_colors()) {
        wattron(window, COLOR_PAIR(2));
    }
    const auto& food = game.get_food();
    mvwaddch(window, food.get_y() + 1, food.get_x() + 1, ACS_BLOCK);
    if (has_colors()) {
        wattroff(window, COLOR_PAIR(2));
    }
    
    // Display scores and generation info
    int y_pos = height + 2;
    mvwprintw(window, y_pos++, 2, "Generation: %d  Step: %d", game.get_generation(), game.get_steps());
    
    // Show top 5 scores
    std::vector<int> top_scores = game.get_top_scores(5);
    mvwprintw(window, y_pos++, 2, "Top scores: ");
    for (size_t i = 0; i < top_scores.size(); i++) {
        mvwprintw(window, y_pos, 2 + i*8, "%d", top_scores[i]);
    }
    
    wrefresh(window);
}

