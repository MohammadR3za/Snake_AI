// training.cpp
#include "training.h"
#include <iostream>
#include <algorithm>
#include <numeric>
#include <chrono>
#include <thread>

Training::Training(int population_size, int generations, 
                  int game_width, int game_height, 
                  int max_steps_without_food,
                  double mutation_rate, double mutation_strength,
                  bool visualize)
    : population_size(NUM_SNAKES), // Always use 10 snakes
      generations(generations),
      game_width(game_width),
      game_height(game_height),
      max_steps_without_food(max_steps_without_food),
      mutation_rate(mutation_rate),
      mutation_strength(mutation_strength),
      visualize(visualize),
      ga(population_size, {24, 16, 4}, mutation_rate, mutation_strength),
      env(game_width, game_height, max_steps_without_food),
      visualizer(game_width, game_height) {
}

void Training::run() {
    // Initialize the genetic algorithm
    ga.initialize_population();
    
    // Vectors to store fitness history
    best_fitness_history.clear();
    avg_fitness_history.clear();
    
    // Training loop
    for (int gen = 0; gen < generations; gen++) {
        std::cout << "Generation " << gen + 1 << "/" << generations << std::endl;
        
        // Create a game with multiple snakes
        Game game(game_width, game_height, NUM_SNAKES);
        game.set_max_steps_without_food(max_steps_without_food);
        game.set_generation(gen + 1);
        
        // Get the current population
        const auto& population = ga.get_population();
        
        // Run the game until all snakes die
        bool game_running = true;
        while (game_running) {
            // Get states for all alive snakes
            auto states = game.get_states_for_ai();
            
            // Determine directions for all snakes
            std::vector<Direction> directions;
            int alive_idx = 0;
            
            for (int i = 0; i < NUM_SNAKES; i++) {
                if (game.is_snake_alive(i)) {
                    // Use the corresponding agent to make a decision
                    directions.push_back(population[i].get_move(states[alive_idx]));
                    alive_idx++;
                } else {
                    // For dead snakes, just provide a default direction (won't be used)
                    directions.push_back(Direction::RIGHT);
                }
            }
            
            // Update the game
            game_running = game.update(directions);
            
            // Visualize if enabled
            if (visualize) {
                visualizer.draw_population_game(game);
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            }
        }
        
        // Calculate fitness for each snake
        std::vector<double> fitness_scores;
        for (int i = 0; i < NUM_SNAKES; i++) {
            double score = game.get_score(i);
            double steps = game.get_steps();
            
            // Fitness formula: score is the main component, steps is a small bonus
            double fitness = score * 10.0 + steps * 0.01;
            fitness_scores.push_back(fitness);
        }
        
        // Set fitness scores for the genetic algorithm
        ga.evaluate_population([&](const SnakeAgent& agent) -> double {
            // This is a dummy function since we're setting fitness scores directly
            return 0.0;
        });
        
        // Store fitness statistics
        double best_fitness = *std::max_element(fitness_scores.begin(), fitness_scores.end());
        double avg_fitness = std::accumulate(fitness_scores.begin(), fitness_scores.end(), 0.0) / NUM_SNAKES;
        
        best_fitness_history.push_back(best_fitness);
        avg_fitness_history.push_back(avg_fitness);
        
        std::cout << "  Best fitness: " << best_fitness << std::endl;
        std::cout << "  Average fitness: " << avg_fitness << std::endl;
        
        // Evolve the population
        ga.evolve();
    }
}

void Training::save_best_agent(const std::string& filename) const {
    ga.save_best_agent(filename);
}
