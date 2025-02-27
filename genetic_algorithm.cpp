#include "genetic_algorithm.h"
#include <algorithm>
#include <iostream>
#include <fstream>
#include <limits>

GeneticAlgorithm::GeneticAlgorithm(size_t population_size, const std::vector<size_t>& network_architecture,
                                 double mutation_rate, double crossover_rate, size_t elite_count)
    : population_size(population_size),
      network_architecture(network_architecture),
      mutation_rate(mutation_rate),
      crossover_rate(crossover_rate),
      elite_count(elite_count),
      rng(std::random_device{}()) {
}

void GeneticAlgorithm::initialize_population() {
    population.clear();
    population.reserve(population_size);
    
    for (size_t i = 0; i < population_size; ++i) {
        Individual individual;
        individual.network = std::make_unique<NeuralNetwork>(network_architecture);
        individual.network->initialize_random();
        population.push_back(std::move(individual));
    }
}

void GeneticAlgorithm::evaluate_fitness(const std::function<double(const NeuralNetwork&)>& fitness_function) {
    for (auto& individual : population) {
        individual.fitness = fitness_function(*individual.network);
    }
}

void GeneticAlgorithm::evolve() {
    // Sort population by fitness (descending)
    std::sort(population.begin(), population.end(), [](const Individual& a, const Individual& b) {
        return a.fitness > b.fitness;
    });
    
    // Keep track of elite individuals
    std::vector<Individual> new_population;
    new_population.reserve(population_size);
    
    // Add elite individuals directly to new population
    for (size_t i = 0; i < elite_count && i < population.size(); ++i) {
        new_population.push_back(std::move(population[i]));
    }
    
    // Create rest of population through selection, crossover, and mutation
    while (new_population.size() < population_size) {
        // Tournament selection
        const Individual& parent1 = select_parent();  // Changed to reference
        const Individual& parent2 = select_parent();  // Changed to reference
        
        // Check diversity between parents
        double parent_distance = calculate_distance(*parent1.network, *parent2.network);
        
        // Apply crossover with probability based on diversity
        double crossover_prob = crossover_rate * (1.0 + parent_distance);
        crossover_prob = std::min(crossover_prob, 0.95); // Cap at 95%
        
        if (dist(rng) < crossover_prob) {
            // Create children through crossover
            Individual child1, child2;
            child1.network = std::make_unique<NeuralNetwork>(network_architecture);
            child2.network = std::make_unique<NeuralNetwork>(network_architecture);
            
            parent1.network->crossover(*parent2.network, *child1.network, *child2.network);
            
            // Apply mutation with adaptive rate based on diversity
            double adaptive_mutation = mutation_rate;
            if (calculate_diversity() < 0.1) {
                // Increase mutation if diversity is low
                adaptive_mutation *= 2.0;
            }
            
            child1.network->mutate(adaptive_mutation, 1.0);
            child2.network->mutate(adaptive_mutation, 1.0);
            
            // Add children to new population
            new_population.push_back(std::move(child1));
            if (new_population.size() < population_size) {
                new_population.push_back(std::move(child2));
            }
        } else {
            // No crossover, just add mutated copies of parents
            Individual child1, child2;
            child1.network = std::make_unique<NeuralNetwork>(*parent1.network);
            child2.network = std::make_unique<NeuralNetwork>(*parent2.network);
            
            child1.network->mutate(mutation_rate * 1.5, 1.0);  // Higher mutation when no crossover
            child2.network->mutate(mutation_rate * 1.5, 1.0);
            
            // Add children to new population
            new_population.push_back(std::move(child1));
            if (new_population.size() < population_size) {
                new_population.push_back(std::move(child2));
            }
        }
    }
    
    // Replace old population with new population
    population = std::move(new_population);
}

const Individual& GeneticAlgorithm::select_parent() const {
    // Tournament selection
    std::uniform_int_distribution<size_t> index_dist(0, population.size() - 1);
    
    // Select tournament participants
    const size_t tournament_size = 3;
    std::vector<size_t> tournament;
    for (size_t i = 0; i < tournament_size; ++i) {
        tournament.push_back(index_dist(rng));
    }
    
    // Find the best individual in the tournament
    size_t best_index = tournament[0];
    for (size_t i = 1; i < tournament.size(); ++i) {
        if (population[tournament[i]].fitness > population[best_index].fitness) {
            best_index = tournament[i];
        }
    }
    
    return population[best_index];
}

const Individual& GeneticAlgorithm::get_best_individual() const {
    auto best_it = std::max_element(population.begin(), population.end(),
                                   [](const Individual& a, const Individual& b) {
                                       return a.fitness < b.fitness;
                                   });
    return *best_it;
}

void GeneticAlgorithm::save_best(const std::string& filename) const {
    const Individual& best = get_best_individual();
    best.network->save_to_file(filename);
}

double GeneticAlgorithm::calculate_diversity() const {
    if (population.empty()) return 0.0;
    
    // Calculate average distance between individuals
    double total_distance = 0.0;
    int count = 0;
    
    // Sample pairs to avoid O(n²) complexity with large populations
    const int max_pairs = 1000;
    const int population_size = static_cast<int>(population.size());
    std::uniform_int_distribution<int> index_dist(0, population_size - 1);
    
    for (int i = 0; i < max_pairs; ++i) {
        int idx1 = index_dist(rng);
        int idx2 = index_dist(rng);
        
        // Ensure we don't compare an individual with itself
        if (idx1 != idx2) {
            total_distance += calculate_distance(*population[idx1].network, *population[idx2].network);
            ++count;
        }
    }
    
    return count > 0 ? total_distance / count : 0.0;
}

double GeneticAlgorithm::calculate_distance(const NeuralNetwork& a, const NeuralNetwork& b) const {
    double distance = 0.0;
    int count = 0;
    
    // Compare weights
    for (size_t layer = 0; layer < a.get_weights().size(); ++layer) {
        for (size_t neuron = 0; neuron < a.get_weights()[layer].size(); ++neuron) {
            for (size_t weight = 0; weight < a.get_weights()[layer][neuron].size(); ++weight) {
                distance += std::abs(a.get_weights()[layer][neuron][weight] - b.get_weights()[layer][neuron][weight]);
                ++count;
            }
        }
    }
    
    // Compare biases
    for (size_t layer = 0; layer < a.get_biases().size(); ++layer) {
        for (size_t neuron = 0; neuron < a.get_biases()[layer].size(); ++neuron) {
            distance += std::abs(a.get_biases()[layer][neuron] - b.get_biases()[layer][neuron]);
            ++count;
        }
    }
    
    return count > 0 ? distance / count : 0.0;
}
