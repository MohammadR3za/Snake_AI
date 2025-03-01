
// neural_network.cpp
#include "neural_network.h"
#include <cmath>
#include <random>
#include <fstream>

NeuralNetwork::NeuralNetwork(const std::vector<int>& topology) : topology(topology) {
    // Initialize random number generator
    std::random_device rd;
    std::mt19937 gen(rd());
    std::normal_distribution<double> dist(0.0, 1.0);
    
    // Create weights and biases
    for (size_t i = 0; i < topology.size() - 1; i++) {
        std::vector<std::vector<double>> layer_weights;
        std::vector<double> layer_biases;
        
        for (int j = 0; j < topology[i + 1]; j++) {
            std::vector<double> neuron_weights;
            
            for (int k = 0; k < topology[i]; k++) {
                // Xavier initialization
                double weight = dist(gen) * sqrt(2.0 / (topology[i] + topology[i + 1]));
                neuron_weights.push_back(weight);
            }
            
            layer_weights.push_back(neuron_weights);
            layer_biases.push_back(dist(gen) * 0.1);
        }
        
        weights.push_back(layer_weights);
        biases.push_back(layer_biases);
    }
}

NeuralNetwork::NeuralNetwork(const std::string& filename) {
    load_from_file(filename);
}

std::vector<double> NeuralNetwork::feed_forward(const std::vector<double>& inputs) const {
    std::vector<double> current_values = inputs;
    
    // Process each layer
    for (size_t i = 0; i < weights.size(); i++) {
        std::vector<double> next_values;
        
        // For each neuron in the next layer
        for (size_t j = 0; j < weights[i].size(); j++) {
            double sum = biases[i][j];
            
            // For each connection to the current layer
            for (size_t k = 0; k < weights[i][j].size(); k++) {
                sum += weights[i][j][k] * current_values[k];
            }
            
            next_values.push_back(activate(sum));
        }
        
        current_values = next_values;
    }
    
    return current_values;
}

// src/neural_network.cpp (partial fix for the set_weights method)
void NeuralNetwork::set_weights(const std::vector<double>& flat_weights) {
    int index = 0;
    
    // Set weights between layers
    for (size_t i = 0; i < weights.size(); i++) {
        for (size_t j = 0; j < weights[i].size(); j++) {
            for (size_t k = 0; k < weights[i][j].size(); k++) {
                if (static_cast<size_t>(index) < flat_weights.size()) {
                    weights[i][j][k] = flat_weights[index];
                }
                index++;
            }
        }
    }
    
    // Set biases
    for (size_t i = 0; i < biases.size(); i++) {
        for (size_t j = 0; j < biases[i].size(); j++) {
            if (static_cast<size_t>(index) < flat_weights.size()) {
                biases[i][j] = flat_weights[index];
            }
            index++;
        }
    }
}

std::vector<double> NeuralNetwork::get_weights() const {
    std::vector<double> flat_weights;
    
    // Get weights
    for (const auto& layer : weights) {
        for (const auto& neuron : layer) {
            for (double weight : neuron) {
                flat_weights.push_back(weight);
            }
        }
    }
    
    // Get biases
    for (const auto& layer : biases) {
        for (double bias : layer) {
            flat_weights.push_back(bias);
        }
    }
    
    return flat_weights;
}

int NeuralNetwork::get_weights_count() const {
    int count = 0;
    
    // Count weights
    for (const auto& layer : weights) {
        for (const auto& neuron : layer) {
            count += neuron.size();
        }
    }
    
    // Count biases
    for (const auto& layer : biases) {
        count += layer.size();
    }
    
    return count;
}

double NeuralNetwork::activate(double x) const {
    // ReLU activation function
    return std::max(0.0, x);
}

double NeuralNetwork::activate_derivative(double x) const {
    // Derivative of ReLU
    return x > 0.0 ? 1.0 : 0.0;
}

bool NeuralNetwork::save_to_file(const std::string& filename) const {
    std::ofstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }
    
    // Save topology
    int topology_size = topology.size();
    file.write(reinterpret_cast<const char*>(&topology_size), sizeof(int));
    for (int size : topology) {
        file.write(reinterpret_cast<const char*>(&size), sizeof(int));
    }
    
    // Save weights
    for (const auto& layer : weights) {
        for (const auto& neuron : layer) {
            for (double weight : neuron) {
                file.write(reinterpret_cast<const char*>(&weight), sizeof(double));
            }
        }
    }
    
    // Save biases
    for (const auto& layer : biases) {
        for (double bias : layer) {
            file.write(reinterpret_cast<const char*>(&bias), sizeof(double));
        }
    }
    
    file.close();
    return true;
}

bool NeuralNetwork::load_from_file(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }
    
    // Load topology
    int topology_size;
    file.read(reinterpret_cast<char*>(&topology_size), sizeof(int));
    
    topology.clear();
    for (int i = 0; i < topology_size; i++) {
        int size;
        file.read(reinterpret_cast<char*>(&size), sizeof(int));
        topology.push_back(size);
    }
    
    // Initialize weights and biases structures
    weights.clear();
    biases.clear();
    
    for (size_t i = 0; i < topology.size() - 1; i++) {
        std::vector<std::vector<double>> layer_weights;
        std::vector<double> layer_biases;
        
        for (int j = 0; j < topology[i + 1]; j++) {
            std::vector<double> neuron_weights;
            
            for (int k = 0; k < topology[i]; k++) {
                double weight;
                file.read(reinterpret_cast<char*>(&weight), sizeof(double));
                neuron_weights.push_back(weight);
            }
            
            layer_weights.push_back(neuron_weights);
        }
        
        weights.push_back(layer_weights);
        
        for (int j = 0; j < topology[i + 1]; j++) {
            double bias;
            file.read(reinterpret_cast<char*>(&bias), sizeof(double));
            layer_biases.push_back(bias);
        }
        
        biases.push_back(layer_biases);
    }
    
    file.close();
    return true;
}
