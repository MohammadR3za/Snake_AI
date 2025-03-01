
// genetic_algorithm.cpp
#include "genetic_algorithm.h"
#include "game.h"
#include <algorithm>
#include <random>
#include <thread>

GeneticAlgorithm::GeneticAlgorithm(int population_size, 
                                  const std::vector<int>& network_topology, 
                                  double mutation_rate, 
                                  double mutation_strength)
    : population_size(population_size), 
      network_topology(network_topology), 
      mutation_rate(mutation_rate), 
      mutation_strength(mutation_strength) {
    
    initialize_population();
}

void GeneticAlgorithm::initialize_population() {
    population.clear();
    fitness_scores.resize(population_size, 0.0);
    
    for (int i = 0; i < population_size; i++) {
        population.push_back(SnakeAgent(network_topology));
    }
}

void GeneticAlgorithm::evaluate_population(const std::function<double(const SnakeAgent&)>& fitness_function) {
    // Evaluate each agent
    for (int i = 0; i < population_size; i++) {
        // If fitness scores are already set (as in our population-based approach),
        // we don't need to recalculate them
        if (i >= fitness_scores.size()) {
            fitness_scores.push_back(fitness_function(population[i]));
        }
    }
}

void GeneticAlgorithm::evaluate_subset(int start, int end, 
                                      const std::function<double(const SnakeAgent&)>& fitness_function) {
    for (int i = start; i < end; i++) {
        fitness_scores[i] = fitness_function(population[i]);
    }
}

void GeneticAlgorithm::evolve_parallel() {
    // Determine number of threads to use
    unsigned int num_threads = std::thread::hardware_concurrency();
    if (num_threads == 0) num_threads = 4;  // Default if hardware_concurrency is not supported
    
    std::vector<std::thread> threads;
    int agents_per_thread = population_size / num_threads;
    
    // Create a local copy of the fitness function for each thread
    auto fitness_function = [this](const SnakeAgent& agent) -> double {
        // Run the agent in multiple games and return the average score
        double total_score = 0.0;
        int num_games = 5;
        
        for (int i = 0; i < num_games; i++) {
            Game game;
            game.set_max_steps_without_food(100);
            
            while (!game.is_game_over()) {
                std::vector<double> state = game.get_state_for_ai();
                Direction move = agent.get_move(state);
                game.update(move);
            }
            
            total_score += game.get_score();
        }
        
        return total_score / num_games;
    };
    
    // Launch threads
    for (unsigned int i = 0; i < num_threads; i++) {
        int start = i * agents_per_thread;
        int end = (i == num_threads - 1) ? population_size : (i + 1) * agents_per_thread;
        threads.push_back(std::thread(&GeneticAlgorithm::evaluate_subset, this, start, end, fitness_function));
    }
    
    // Join threads
    for (auto& thread : threads) {
        thread.join();
    }
    
    // Perform selection, crossover, and mutation
    selection();
    crossover();
    mutation();
}

void GeneticAlgorithm::evolve() {
    // Sort population by fitness
    selection();
    crossover();
    mutation();

    // Clear fitness scores for the next generation
    fitness_scores.clear();
}

void GeneticAlgorithm::selection() {
    // Create pairs of (agent, fitness)
    std::vector<std::pair<SnakeAgent, double>> population_with_fitness;
    for (size_t i = 0; i < population.size(); i++) {
        population_with_fitness.push_back({population[i], fitness_scores[i]});
    }
    
    // Sort by fitness (descending)
    std::sort(population_with_fitness.begin(), population_with_fitness.end(), 
              [](const auto& a, const auto& b) { return a.second > b.second; });
    
    // Keep only the best half
    std::vector<SnakeAgent> new_population;
    for (size_t i = 0; i < population_size / 2; i++) {
        new_population.push_back(population_with_fitness[i].first);
    }
    
    population = new_population;
}

void GeneticAlgorithm::crossover() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(0, population.size() - 1);
    std::uniform_real_distribution<> crossover_point(0.0, 1.0);
    
    std::vector<SnakeAgent> new_population = population;
    
    while (new_population.size() < population_size) {
        // Select two parents
        int parent1_idx = dist(gen);
        int parent2_idx = dist(gen);
        
        // Ensure different parents
        while (parent2_idx == parent1_idx) {
            parent2_idx = dist(gen);
        }
        
        SnakeAgent parent1 = population[parent1_idx];
        SnakeAgent parent2 = population[parent2_idx];
        
        // Create child with parent1's neural network
        SnakeAgent child(parent1.get_brain());
        
        // Get weights from both parents
        std::vector<double> weights1 = parent1.get_brain().get_weights();
        std::vector<double> weights2 = parent2.get_brain().get_weights();
        std::vector<double> child_weights = weights1;
        
        // Perform crossover
        double point = crossover_point(gen);
        for (size_t i = 0; i < weights1.size(); i++) {
            if (static_cast<double>(i) / weights1.size() > point) {
                child_weights[i] = weights2[i];
            }
        }
        
        // Update child's weights
        NeuralNetwork child_brain = child.get_brain();
        child_brain.set_weights(child_weights);
        child.set_brain(child_brain);
        
        new_population.push_back(child);
    }
    
    population = new_population;
}

void GeneticAlgorithm::mutation() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dist(0.0, 1.0);
    std::normal_distribution<> mutation(0.0, mutation_strength);
    
    for (auto& agent : population) {
        NeuralNetwork brain = agent.get_brain();
        std::vector<double> weights = brain.get_weights();
        
        for (double& weight : weights) {
            if (dist(gen) < mutation_rate) {
                weight += mutation(gen);
            }
        }
        
        brain.set_weights(weights);
        agent.set_brain(brain);
    }
}

SnakeAgent GeneticAlgorithm::get_best_agent() const {
    if (population.empty()) {
        return SnakeAgent(network_topology);
    }
    
    // Find agent with highest fitness
    int best_idx = 0;
    double best_fitness = fitness_scores[0];
    
    for (size_t i = 1; i < fitness_scores.size(); i++) {
        if (fitness_scores[i] > best_fitness) {
            best_fitness = fitness_scores[i];
            best_idx = i;
        }
    }
    
    return population[best_idx];
}

void GeneticAlgorithm::save_best_agent(const std::string& filename) const {
    SnakeAgent best_agent = get_best_agent();
    best_agent.save_to_file(filename);
}
