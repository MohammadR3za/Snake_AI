#ifndef NEURAL_NETWORK_H
#define NEURAL_NETWORK_H

#include <vector>
#include <string>
#include <random>

class NeuralNetwork {
public:
    NeuralNetwork(const std::vector<size_t>& layer_sizes);
    NeuralNetwork(const NeuralNetwork& other);
    
    void initialize_random();
    std::vector<double> forward(const std::vector<double>& inputs) const;
    void mutate(double mutation_rate, double mutation_strength);
    void crossover(const NeuralNetwork& other, NeuralNetwork& child1, NeuralNetwork& child2) const;
    
    bool save_to_file(const std::string& filename) const;
    bool load_from_file(const std::string& filename);
    
    // Getters for weights and biases (for diversity calculation)
    const std::vector<std::vector<std::vector<double>>>& get_weights() const { return weights; }
    const std::vector<std::vector<double>>& get_biases() const { return biases; }
    
private:
    double activate(double x) const;  // Activation function (ReLU)
    
    std::vector<size_t> layer_sizes;
    std::vector<std::vector<std::vector<double>>> weights;  // [layer][neuron][weight]
    std::vector<std::vector<double>> biases;  // [layer][neuron]
    
    static std::mt19937 rng;
    static std::normal_distribution<double> normal_dist;
    static std::uniform_real_distribution<double> uniform_dist;
};

#endif // NEURAL_NETWORK_H
