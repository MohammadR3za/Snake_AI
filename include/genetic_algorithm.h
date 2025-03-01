
// genetic_algorithm.h
#ifndef GENETIC_ALGORITHM_H
#define GENETIC_ALGORITHM_H

#include "snake_agent.h"
#include <vector>
#include <functional>
#include <thread>

class GeneticAlgorithm {
public:
    GeneticAlgorithm(int population_size, 
                     const std::vector<int>& network_topology, 
                     double mutation_rate = 0.1, 
                     double mutation_strength = 0.5);
    
    void initialize_population();
    void evolve();
    void evolve_parallel();  // Added parallel evolution method
    
    // Added method to evaluate population with a fitness function
    void evaluate_population(const std::function<double(const SnakeAgent&)>& fitness_function);
    
    // Added method to get the population
    const std::vector<SnakeAgent>& get_population() const { return population; }
    const std::vector<double>& get_fitness_scores() const { return fitness_scores; }
    SnakeAgent get_best_agent() const;
    
    void save_best_agent(const std::string& filename) const;
    
private:
    int population_size;
    std::vector<int> network_topology;
    double mutation_rate;
    double mutation_strength;
    
    std::vector<SnakeAgent> population;
    std::vector<double> fitness_scores;
    
    void selection();
    void crossover();
    void mutation();
    
    // Helper method for parallel evaluation
    void evaluate_subset(int start, int end, const std::function<double(const SnakeAgent&)>& fitness_function);
};

#endif // GENETIC_ALGORITHM_H
