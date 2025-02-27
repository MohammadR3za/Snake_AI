#include "genetic_algorithm.h"
#include "neural_network.h"
#include "config.h"
#include "common.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <chrono>
#include <filesystem>
#include <fstream>

// Function to evaluate the fitness of a neural network
double evaluate_fitness(const NeuralNetwork& network) {
    // Example fitness function - replace with your actual evaluation
    // This could involve running simulations, game playing, etc.
    
    // For demonstration, let's just sum the absolute values of all weights
    double sum = 0.0;
    for (const auto& layer : network.get_weights()) {
        for (const auto& neuron : layer) {
            for (double weight : neuron) {
                sum += std::abs(weight);
            }
        }
    }
    
    // Normalize to [0, 1] range
    return 1.0 / (1.0 + sum);
}

int main(int argc, char* argv[]) {
    // Load configuration
    Config config;
    if (argc > 1) {
        config.load_from_file(argv[1]);
    }
    
    // Create output directory if it doesn't exist
    std::filesystem::create_directories(config.output_dir);
    
    // Initialize genetic algorithm
    GeneticAlgorithm ga(
        config.population_size,
        config.network_architecture,
        config.mutation_rate,
        config.crossover_rate,
        config.elite_count
    );
    
    // Initialize population
    ga.initialize_population();
    
    // Evolution loop
    std::ofstream log_file(config.output_dir + "/evolution_log.csv");
    log_file << "Generation,Best Fitness,Average Fitness,Diversity\n";
    
    for (size_t generation = 0; generation < config.generations; ++generation) {
        // Evaluate fitness of all individuals
        ga.evaluate_fitness(evaluate_fitness);
        
        // Get statistics
        double total_fitness = 0.0;
        double best_fitness = 0.0;
        
        for (const auto& individual : ga.get_population()) {
            total_fitness += individual.fitness;
            best_fitness = std::max(best_fitness, individual.fitness);
        }
        
        double avg_fitness = total_fitness / ga.get_population().size();
        double diversity = ga.calculate_diversity();
        
        // Log progress
        std::cout << "Generation " << generation + 1 << "/" << config.generations
                  << " | Best Fitness: " << std::fixed << std::setprecision(4) << best_fitness
                  << " | Avg Fitness: " << avg_fitness
                  << " | Diversity: " << diversity << std::endl;
        
        log_file << generation + 1 << ","
                 << best_fitness << ","
                 << avg_fitness << ","
                 << diversity << std::endl;
        
        // Save best individual periodically
        if ((generation + 1) % config.save_interval == 0 || generation == config.generations - 1) {
            const Individual& best = ga.get_best_individual();
            
            // Create filename with generation and fitness
            std::stringstream ss;
            ss << config.output_dir << "/best_network_gen" << generation + 1 
               << "_fit" << std::fixed << std::setprecision(2) << best.fitness 
               << ".bin";
            
            ga.save_best(ss.str());
            std::cout << "Saved best network to " << ss.str() << std::endl;
        }
        
        // Evolve population (except on last generation)
        if (generation < config.generations - 1) {
            ga.evolve();
        }
    }
    
    log_file.close();
    std::cout << "Evolution complete. Results saved to " << config.output_dir << std::endl;
    
    return 0;
}
