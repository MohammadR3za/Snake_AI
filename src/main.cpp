// main.cpp
#include "game.h"
#include "snake_agent.h"
#include "training.h"
#include "training_visualizer.h"
#include <iostream>
#include <string>
#include <cstring>

void print_usage(const char* program_name) {
    std::cout << "Usage: " << program_name << " [mode] [options]" << std::endl;
    std::cout << "Modes:" << std::endl;
    std::cout << "  play                - Play the game manually" << std::endl;
    std::cout << "  train               - Train a new AI agent" << std::endl;
    std::cout << "  test [model_file]   - Test a trained AI agent" << std::endl;
    std::cout << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  --width [width]     - Set game width (default: 20)" << std::endl;
    std::cout << "  --height [height]   - Set game height (default: 20)" << std::endl;
    std::cout << "  --population [size] - Set population size for training (default: 100)" << std::endl;
    std::cout << "  --generations [num] - Set number of generations for training (default: 100)" << std::endl;
    std::cout << "  --mutation [rate]   - Set mutation rate (default: 0.1)" << std::endl;
    std::cout << "  --strength [value]  - Set mutation strength (default: 0.5)" << std::endl;
    std::cout << "  --no-visual         - Disable visualization during training" << std::endl;
}

int main(int argc, char* argv[]) {
    // Default parameters
    std::string mode = "play";
    std::string model_file = "";
    int width = 20;
    int height = 20;
    int population_size = 100;
    int generations = 100;
    double mutation_rate = 0.1;
    double mutation_strength = 0.5;
    bool visualize = true;

    // Parse command line arguments
    if (argc > 1) {
        mode = argv[1];
        
        if (mode == "test" && argc > 2) {
            model_file = argv[2];
        }
        
        for (int i = 1; i < argc; i++) {
            if (strcmp(argv[i], "--width") == 0 && i + 1 < argc) {
                width = std::stoi(argv[i + 1]);
            }
            else if (strcmp(argv[i], "--height") == 0 && i + 1 < argc) {
                height = std::stoi(argv[i + 1]);
            }
            else if (strcmp(argv[i], "--population") == 0 && i + 1 < argc) {
                population_size = std::stoi(argv[i + 1]);
            }
            else if (strcmp(argv[i], "--generations") == 0 && i + 1 < argc) {
                generations = std::stoi(argv[i + 1]);
            }
            else if (strcmp(argv[i], "--mutation") == 0 && i + 1 < argc) {
                mutation_rate = std::stod(argv[i + 1]);
            }
            else if (strcmp(argv[i], "--strength") == 0 && i + 1 < argc) {
                mutation_strength = std::stod(argv[i + 1]);
            }
            else if (strcmp(argv[i], "--no-visual") == 0) {
                visualize = false;
            }
        }
    }
    
    // Validate parameters
    if (width < 10) width = 10;
    if (height < 10) height = 10;
    if (population_size < 10) population_size = 10;
    if (generations < 1) generations = 1;
    if (mutation_rate < 0.0) mutation_rate = 0.0;
    if (mutation_rate > 1.0) mutation_rate = 1.0;
    if (mutation_strength < 0.0) mutation_strength = 0.0;
    if (mutation_strength > 1.0) mutation_strength = 1.0;
    
    // Handle different modes
    if (mode == "play") {
        // Manual play mode
        Game game(width, height);
        TrainingVisualizer visualizer(width, height);
        
        Direction current_dir = Direction::RIGHT;
        bool running = true;
        
        while (running) {
            // Get user input
            int ch = getch();
            switch (ch) {
                case KEY_UP:
                    if (current_dir != Direction::DOWN)
                        current_dir = Direction::UP;
                    break;
                case KEY_RIGHT:
                    if (current_dir != Direction::LEFT)
                        current_dir = Direction::RIGHT;
                    break;
                case KEY_DOWN:
                    if (current_dir != Direction::UP)
                        current_dir = Direction::DOWN;
                    break;
                case KEY_LEFT:
                    if (current_dir != Direction::RIGHT)
                        current_dir = Direction::LEFT;
                    break;
                case 'q':
                case 'Q':
                    running = false;
                    break;
            }
            
            // Update game
            running = running && game.update(current_dir);
            
            // Draw game
            visualizer.draw_game(game);
            
            // Delay for gameplay speed
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        
        return 0;
    }
    else if (mode == "train") {
        // Training mode
        Training training(population_size, generations, width, height, 100, 
                         mutation_rate, mutation_strength, visualize);
        training.run();
        
        // Save the best agent
        training.save_best_agent("best_agent.model");
        
        return 0;
    }
    else if (mode == "test") {
        // Testing mode
        if (model_file.empty()) {
            std::cout << "Error: No model file specified for testing." << std::endl;
            print_usage(argv[0]);
            return 1;
        }
        
        // Load the agent
        SnakeAgent agent({24, 16, 4});
        if (!agent.load_from_file(model_file)) {
            std::cout << "Error: Failed to load model from " << model_file << std::endl;
            return 1;
        }
        
        // Test the agent
        TrainingVisualizer visualizer(width, height);
        visualizer.visualize_agent(agent, 5, 100);  // Play 5 games with 100ms delay
        
        return 0;
    }
    else {
        // Unknown mode
        std::cout << "Error: Unknown mode '" << mode << "'" << std::endl;
        print_usage(argv[0]);
        return 1;
    }
}
