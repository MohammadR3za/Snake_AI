#ifndef GENETIC_ALGORITHM_H
#define GENETIC_ALGORITHM_H

#include "neural_network.h"
#include <vector>
#include <memory>
#include <random>
#include <functional>

struct Individual {
    std::unique_ptr<NeuralNetwork> network;
    double fitness = 0.0;
};

class GeneticAlgorithm {
public:
    GeneticAlgorithm(size_t population_size, const std::vector<size_t>& network_architecture,
                   double mutation_rate, double crossover_rate, size_t elite_count);
    
    void initialize_population();
    void evaluate_fitness(const std::function<double(const NeuralNetwork&)>& fitness_function);
    void evolve();
    
    const Individual& get_best_individual() const;
    void save_best(const std::string& filename) const;
    double calculate_diversity() const;
    
    const std::vector<Individual>& get_population() const { return population; }
    
private:
    const Individual& select_parent() const;  // Changed to return a reference
    double calculate_distance(const NeuralNetwork& a, const NeuralNetwork& b) const;
    
    size_t population_size;
    std::vector<size_t> network_architecture;
    double mutation_rate;
    double crossover_rate;
    size_t elite_count;
    
    std::vector<Individual> population;
    mutable std::mt19937 rng;
    mutable std::uniform_real_distribution<double> dist{0.0, 1.0};
};

#endif // GENETIC_ALGORITHM_H
