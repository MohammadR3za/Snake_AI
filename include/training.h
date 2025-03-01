
// training.h
#ifndef TRAINING_H
#define TRAINING_H

#include "genetic_algorithm.h"
#include "game_environment.h"
#include "training_visualizer.h"
#include <string>
#include <vector>

class Training {
public:
    // Added constant for number of snakes
    static const int NUM_SNAKES = 10;
    
    Training(int population_size = 100,
             int generations = 100,
             int game_width = 20,
             int game_height = 20,
             int max_steps_without_food = 100,
             double mutation_rate = 0.1,
             double mutation_strength = 0.5,
             bool visualize = true);
    
    void run();
    void save_best_agent(const std::string& filename) const;
    
    // Getters for training statistics
    const std::vector<double>& get_best_fitness_history() const { return best_fitness_history; }
    const std::vector<double>& get_avg_fitness_history() const { return avg_fitness_history; }
    
private:
    int population_size;
    int generations;
    int game_width;
    int game_height;
    int max_steps_without_food;
    double mutation_rate;
    double mutation_strength;
    bool visualize;
    
    GeneticAlgorithm ga;
    GameEnvironment env;
    TrainingVisualizer visualizer;
    
    std::vector<double> best_fitness_history;
    std::vector<double> avg_fitness_history;
};

#endif // TRAINING_H
