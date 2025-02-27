#include "snake.h"
#include <iostream>
#include <string>
#include <cstdlib>

void print_usage() {
    std::cout << "Usage: snake_ai [mode] [options]\n"
              << "Modes:\n"
              << "  play      - Play the game manually\n"
              << "  ai        - Watch the AI play\n"
              << "  train     - Train the AI\n"
              << "\n"
              << "Options:\n"
              << "  --width=N           - Set board width (default: 20)\n"
              << "  --height=N          - Set board height (default: 15)\n"
              << "  --episodes=N        - Number of training episodes (default: 1000)\n"
              << "  --steps=N           - Maximum steps per episode (default: 1000)\n"
              << "  --model=FILE        - Load/save model from/to FILE\n"
              << "  --config=FILE       - Load configuration from FILE\n"
              << "  --no-visual         - Disable visualization\n"
              << "  --delay=N           - Animation delay in milliseconds (default: 100)\n"
              << std::endl;
}

int main(int argc, char* argv[]) {
    // Default settings
    GameMode mode = GameMode::MANUAL;
    unsigned int width = 20;
    unsigned int height = 15;
    int episodes = 1000;
    int max_steps = 1000;
    bool visualize = true;
    int animation_delay = 100;
    std::string model_file;
    std::string config_file;
    
    // Parse command line arguments
    if (argc < 2) {
        print_usage();
        return 1;
    }
    
    // Parse mode
    std::string mode_str = argv[1];
    if (mode_str == "play") {
        mode = GameMode::MANUAL;
    } else if (mode_str == "ai") {
        mode = GameMode::AI;
    } else if (mode_str == "train") {
        mode = GameMode::TRAINING;
    } else {
        std::cerr << "Error: Unknown mode '" << mode_str << "'\n";
        print_usage();
        return 1;
    }
    
    // Parse options
    for (int i = 2; i < argc; ++i) {
        std::string arg = argv[i];
        
        if (arg.substr(0, 8) == "--width=") {
            width = std::stoi(arg.substr(8));
        } else if (arg.substr(0, 9) == "--height=") {
            height = std::stoi(arg.substr(9));
        } else if (arg.substr(0, 11) == "--episodes=") {
            episodes = std::stoi(arg.substr(11));
        } else if (arg.substr(0, 8) == "--steps=") {
            max_steps = std::stoi(arg.substr(8));
        } else if (arg.substr(0, 8) == "--model=") {
            model_file = arg.substr(8);
        } else if (arg.substr(0, 9) == "--config=") {
            config_file = arg.substr(9);
        } else if (arg == "--no-visual") {
            visualize = false;
        } else if (arg.substr(0, 8) == "--delay=") {
            animation_delay = std::stoi(arg.substr(8));
        } else {
            std::cerr << "Error: Unknown option '" << arg << "'\n";
            print_usage();
            return 1;
        }
    }
    
    // Create the game
    Snake game(width, height, mode);
    
    // Load configuration if specified
    if (!config_file.empty()) {
        if (!game.load_config(config_file)) {
            std::cerr << "Error: Failed to load configuration from " << config_file << std::endl;
            return 1;
        }
    }
    
    // Load model if specified
    if (!model_file.empty() && (mode == GameMode::AI || mode == GameMode::TRAINING)) {
        if (!game.load_model(model_file)) {
            std::cerr << "Error: Failed to load model from " << model_file << std::endl;
            if (mode == GameMode::AI) {
                return 1;
            }
            // In training mode, we can continue with a fresh model
            std::cout << "Starting with a fresh model..." << std::endl;
        }
    }
    
    // Run the game
    if (mode == GameMode::TRAINING) {
        game.train(episodes, max_steps);
        
        // Save the trained model if a file was specified
        if (!model_file.empty()) {
            if (game.save_model(model_file)) {
                std::cout << "Model saved to " << model_file << std::endl;
            } else {
                std::cerr << "Error: Failed to save model to " << model_file << std::endl;
                return 1;
            }
        }
    } else {
        game.play();
    }
    
    return 0;
}
