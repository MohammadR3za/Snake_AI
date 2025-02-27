#include "neural_network.h"
#include <cmath>
#include <fstream>
#include <iostream>
#include <algorithm>

// Initialize static members
std::mt19937 NeuralNetwork::rng(std::random_device{}());
std::normal_distribution<double> NeuralNetwork::normal_dist(0.0, 1.0);
std::uniform_real_distribution<double> NeuralNetwork::uniform_dist(0.0, 1.0);

NeuralNetwork::NeuralNetwork(const std::vector<size_t>& layer_sizes)
    : layer_sizes(layer_sizes) {
    
    // Initialize weights and biases with zeros
    weights.resize(layer_sizes.size() - 1);
    biases.resize(layer_sizes.size() - 1);
    
    for (size_t layer = 0; layer < layer_sizes.size() - 1; ++layer) {
        size_t inputs = layer_sizes[layer];
        size_t outputs = layer_sizes[layer + 1];
        
        weights[layer].resize(outputs);
        biases[layer].resize(outputs, 0.0);
        
        for (size_t neuron = 0; neuron < outputs; ++neuron) {
            weights[layer][neuron].resize(inputs, 0.0);
        }
    }
}

NeuralNetwork::NeuralNetwork(const NeuralNetwork& other)
    : layer_sizes(other.layer_sizes) {
    
    // Deep copy weights and biases
    weights.resize(other.weights.size());
    biases.resize(other.biases.size());
    
    for (size_t layer = 0; layer < other.weights.size(); ++layer) {
        weights[layer].resize(other.weights[layer].size());
        biases[layer].resize(other.biases[layer].size());
        
        for (size_t neuron = 0; neuron < other.weights[layer].size(); ++neuron) {
            weights[layer][neuron] = other.weights[layer][neuron];
            biases[layer][neuron] = other.biases[layer][neuron];
        }
    }
}

void NeuralNetwork::initialize_random() {
    // Xavier/Glorot initialization for better training
    for (size_t layer = 0; layer < layer_sizes.size() - 1; ++layer) {
        size_t inputs = layer_sizes[layer];
        size_t outputs = layer_sizes[layer + 1];
        
        // Calculate standard deviation for Xavier initialization
        double std_dev = std::sqrt(2.0 / (inputs + outputs));
        
        for (size_t neuron = 0; neuron < outputs; ++neuron) {
            // Initialize biases with small random values
            biases[layer][neuron] = normal_dist(rng) * 0.1;
            
            // Initialize weights with Xavier/Glorot initialization
            for (size_t weight = 0; weight < inputs; ++weight) {
                weights[layer][neuron][weight] = normal_dist(rng) * std_dev;
            }
        }
    }
}

std::vector<double> NeuralNetwork::forward(const std::vector<double>& inputs) const {
    // Check input size
    if (inputs.size() != layer_sizes.front()) {
        throw std::runtime_error("Input size does not match network input layer size");
    }
    
    // Forward propagation
    std::vector<double> current_outputs = inputs;
    
    for (size_t layer = 0; layer < weights.size(); ++layer) {
        std::vector<double> next_outputs(weights[layer].size(), 0.0);
        
        for (size_t neuron = 0; neuron < weights[layer].size(); ++neuron) {
            double sum = biases[layer][neuron];
            
            for (size_t input = 0; input < current_outputs.size(); ++input) {
                sum += weights[layer][neuron][input] * current_outputs[input];
            }
            
            // Apply activation function (except for output layer)
            if (layer < weights.size() - 1) {
                next_outputs[neuron] = activate(sum);
            } else {
                // Use sigmoid for output layer to get values between 0 and 1
                next_outputs[neuron] = 1.0 / (1.0 + std::exp(-sum));
            }
        }
        
        current_outputs = next_outputs;
    }
    
    return current_outputs;
}

void NeuralNetwork::mutate(double mutation_rate, double mutation_strength) {
    // Mutate weights
    for (auto& layer : weights) {
        for (auto& neuron : layer) {
            for (auto& weight : neuron) {
                if (uniform_dist(rng) < mutation_rate) {
                    // Apply mutation with given strength
                    weight += normal_dist(rng) * mutation_strength;
                }
            }
        }
    }
    
    // Mutate biases
    for (auto& layer : biases) {
        for (auto& bias : layer) {
            if (uniform_dist(rng) < mutation_rate) {
                // Apply mutation with given strength
                bias += normal_dist(rng) * mutation_strength;
            }
        }
    }
}

void NeuralNetwork::crossover(const NeuralNetwork& other, NeuralNetwork& child1, NeuralNetwork& child2) const {
    // Ensure all networks have the same architecture
    for (size_t layer = 0; layer < weights.size(); ++layer) {
        for (size_t neuron = 0; neuron < weights[layer].size(); ++neuron) {
            // Different crossover methods for better diversity
            
            // Method selection based on layer
            int crossover_method = (layer % 3); // Cycle between 3 methods
            
            if (crossover_method == 0) {
                // Method 1: Single point crossover per neuron
                size_t crossover_point = std::uniform_int_distribution<size_t>(
                    0, weights[layer][neuron].size())(rng);
                
                for (size_t weight = 0; weight < weights[layer][neuron].size(); ++weight) {
                    if (weight < crossover_point) {
                        child1.weights[layer][neuron][weight] = weights[layer][neuron][weight];
                        child2.weights[layer][neuron][weight] = other.weights[layer][neuron][weight];
                    } else {
                        child1.weights[layer][neuron][weight] = other.weights[layer][neuron][weight];
                        child2.weights[layer][neuron][weight] = weights[layer][neuron][weight];
                    }
                }
            } else if (crossover_method == 1) {
                // Method 2: Uniform crossover
                for (size_t weight = 0; weight < weights[layer][neuron].size(); ++weight) {
                    if (uniform_dist(rng) < 0.5) {
                        child1.weights[layer][neuron][weight] = weights[layer][neuron][weight];
                        child2.weights[layer][neuron][weight] = other.weights[layer][neuron][weight];
                    } else {
                        child1.weights[layer][neuron][weight] = other.weights[layer][neuron][weight];
                        child2.weights[layer][neuron][weight] = weights[layer][neuron][weight];
                    }
                }
            } else {
                // Method 3: Arithmetic crossover (blend)
                double alpha = uniform_dist(rng);
                for (size_t weight = 0; weight < weights[layer][neuron].size(); ++weight) {
                    child1.weights[layer][neuron][weight] = alpha * weights[layer][neuron][weight] + 
                                                          (1 - alpha) * other.weights[layer][neuron][weight];
                    child2.weights[layer][neuron][weight] = (1 - alpha) * weights[layer][neuron][weight] + 
                                                          alpha * other.weights[layer][neuron][weight];
                }
            }
            
            // Crossover for biases (always uniform)
            if (uniform_dist(rng) < 0.5) {
                child1.biases[layer][neuron] = biases[layer][neuron];
                child2.biases[layer][neuron] = other.biases[layer][neuron];
            } else {
                child1.biases[layer][neuron] = other.biases[layer][neuron];
                child2.biases[layer][neuron] = biases[layer][neuron];
            }
        }
    }
}

bool NeuralNetwork::save_to_file(const std::string& filename) const {
    std::ofstream file(filename, std::ios::binary);
    if (!file) {
        std::cerr << "Failed to open file for writing: " << filename << std::endl;
        return false;
    }
    
    // Save network architecture
    size_t num_layers = layer_sizes.size();
    file.write(reinterpret_cast<const char*>(&num_layers), sizeof(num_layers));
    
    for (const auto& size : layer_sizes) {
        file.write(reinterpret_cast<const char*>(&size), sizeof(size));
    }
    
    // Save weights
    for (const auto& layer : weights) {
        for (const auto& neuron : layer) {
            for (const auto& weight : neuron) {
                file.write(reinterpret_cast<const char*>(&weight), sizeof(weight));
            }
        }
    }
    
    // Save biases
    for (const auto& layer : biases) {
        for (const auto& bias : layer) {
            file.write(reinterpret_cast<const char*>(&bias), sizeof(bias));
        }
    }
    
    return file.good();
}

bool NeuralNetwork::load_from_file(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        std::cerr << "Failed to open file for reading: " << filename << std::endl;
        return false;
    }
    
    // Load network architecture
    size_t num_layers;
    file.read(reinterpret_cast<char*>(&num_layers), sizeof(num_layers));
    
    std::vector<size_t> loaded_layer_sizes(num_layers);
    for (auto& size : loaded_layer_sizes) {
        file.read(reinterpret_cast<char*>(&size), sizeof(size));
    }
    
    // Check if architecture matches
    if (loaded_layer_sizes != layer_sizes) {
        std::cerr << "Network architecture in file does not match current network" << std::endl;
        return false;
    }
    
    // Load weights
    for (auto& layer : weights) {
        for (auto& neuron : layer) {
            for (auto& weight : neuron) {
                file.read(reinterpret_cast<char*>(&weight), sizeof(weight));
            }
        }
    }
    
    // Load biases
    for (auto& layer : biases) {
        for (auto& bias : layer) {
            file.read(reinterpret_cast<char*>(&bias), sizeof(bias));
        }
    }
    
    return file.good();
}

double NeuralNetwork::activate(double x) const {
    // ReLU activation function
    return std::max(0.0, x);
}
