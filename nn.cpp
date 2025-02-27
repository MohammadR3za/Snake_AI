#include "nn.h"
#include <cmath>
#include <random>
#include <fstream>
#include <iostream>
#include <algorithm>

NeuralNetwork::NeuralNetwork(const std::vector<int>& layer_sizes, 
                             double learning_rate,
                             ActivationFunction hidden_activation,
                             ActivationFunction output_activation)
    : layer_sizes(layer_sizes),
      learning_rate(learning_rate),
      hidden_activation(hidden_activation),
      output_activation(output_activation)
{
    // Initialize random number generator
    std::random_device rd;
    std::mt19937 gen(rd());
    
    // Xavier/Glorot initialization for weights
    weights.resize(layer_sizes.size() - 1);
    biases.resize(layer_sizes.size() - 1);
    
    for (size_t i = 0; i < layer_sizes.size() - 1; ++i) {
        int fan_in = layer_sizes[i];
        int fan_out = layer_sizes[i + 1];
        double limit = std::sqrt(6.0 / (fan_in + fan_out));
        
        std::uniform_real_distribution<double> dist(-limit, limit);
        
        // Initialize weights for this layer
        weights[i].resize(fan_out);
        for (int j = 0; j < fan_out; ++j) {
            weights[i][j].resize(fan_in);
            for (int k = 0; k < fan_in; ++k) {
                weights[i][j][k] = dist(gen);
            }
        }
        
        // Initialize biases for this layer
        biases[i].resize(fan_out, 0.0);
    }
}

double NeuralNetwork::activate(double x, ActivationFunction func) const {
    switch (func) {
        case ActivationFunction::SIGMOID:
            return 1.0 / (1.0 + std::exp(-x));
        case ActivationFunction::TANH:
            return std::tanh(x);
        case ActivationFunction::RELU:
            return std::max(0.0, x);
        case ActivationFunction::LINEAR:
            return x;
        default:
            return x;
    }
}

double NeuralNetwork::activate_derivative(double x, ActivationFunction func) const {
    switch (func) {
        case ActivationFunction::SIGMOID: {
            double s = activate(x, ActivationFunction::SIGMOID);
            return s * (1.0 - s);
        }
        case ActivationFunction::TANH: {
            double t = std::tanh(x);
            return 1.0 - t * t;
        }
        case ActivationFunction::RELU:
            return x > 0.0 ? 1.0 : 0.0;
        case ActivationFunction::LINEAR:
            return 1.0;
        default:
            return 1.0;
    }
}

std::vector<std::vector<double>> NeuralNetwork::forward_pass(const std::vector<double>& input) {
    std::vector<std::vector<double>> activations;
    activations.push_back(input);
    
    for (size_t i = 0; i < weights.size(); ++i) {
        std::vector<double> layer_output(weights[i].size(), 0.0);
        
        // For each neuron in this layer
        for (size_t j = 0; j < weights[i].size(); ++j) {
            // Compute weighted sum of inputs
            double sum = biases[i][j];
            for (size_t k = 0; k < weights[i][j].size(); ++k) {
                sum += weights[i][j][k] * activations.back()[k];
            }
            
            // Apply activation function
            if (i < weights.size() - 1) {
                // Hidden layer
                layer_output[j] = activate(sum, hidden_activation);
            } else {
                // Output layer
                layer_output[j] = activate(sum, output_activation);
            }
        }
        
        activations.push_back(layer_output);
    }
    
    return activations;
}

std::vector<double> NeuralNetwork::predict(const std::vector<double>& input) {
    // Ensure input size matches first layer
    if (input.size() != layer_sizes[0]) {
        std::cerr << "Error: Input size " << input.size() 
                  << " doesn't match network input size " << layer_sizes[0] << std::endl;
        return std::vector<double>(layer_sizes.back(), 0.0);
    }
    
    std::vector<std::vector<double>> activations = forward_pass(input);
    return activations.back();
}

void NeuralNetwork::train(const std::vector<std::vector<double>>& inputs, 
                         const std::vector<std::vector<double>>& targets,
                         int epochs) {
    if (inputs.empty() || targets.empty() || inputs.size() != targets.size()) {
        std::cerr << "Error: Invalid training data" << std::endl;
        return;
    }
    
    for (int epoch = 0; epoch < epochs; ++epoch) {
        // Mini-batch stochastic gradient descent
        // For simplicity, we'll use the entire dataset as one batch
        
        // Accumulate gradients
        std::vector<std::vector<std::vector<double>>> weight_gradients(weights.size());
        std::vector<std::vector<double>> bias_gradients(biases.size());
        
        // Initialize gradients with zeros
        for (size_t i = 0; i < weights.size(); ++i) {
            weight_gradients[i].resize(weights[i].size());
            for (size_t j = 0; j < weights[i].size(); ++j) {
                weight_gradients[i][j].resize(weights[i][j].size(), 0.0);
            }
            bias_gradients[i].resize(biases[i].size(), 0.0);
        }
        
        // Process each training example
        for (size_t example = 0; example < inputs.size(); ++example) {
            const std::vector<double>& input = inputs[example];
            const std::vector<double>& target = targets[example];
            
            // Forward pass
            std::vector<std::vector<double>> activations = forward_pass(input);
            
            // Backward pass
            // Calculate output layer error
            std::vector<double> errors = activations.back();
            for (size_t i = 0; i < errors.size(); ++i) {
                errors[i] -= target[i];
            }
            
            // Backpropagate error and update gradients
            for (int layer = weights.size() - 1; layer >= 0; --layer) {
                std::vector<double> next_errors(layer_sizes[layer], 0.0);
                
                // For each neuron in this layer
                for (size_t j = 0; j < weights[layer].size(); ++j) {
                    // Get activation derivative
                    double delta;
                    if (layer == weights.size() - 1) {
                        // Output layer
                        delta = errors[j];
                        if (output_activation != ActivationFunction::LINEAR) {
                            // For non-linear output, multiply by derivative
                            double out = activations[layer + 1][j];
                            delta *= activate_derivative(out, output_activation);
                        }
                    } else {
                        // Hidden layer
                        delta = errors[j];
                        if (hidden_activation != ActivationFunction::LINEAR) {
                            // For non-linear hidden, multiply by derivative
                            double out = activations[layer + 1][j];
                            delta *= activate_derivative(out, hidden_activation);
                        }
                    }
                    
                    // Update bias gradient
                    bias_gradients[layer][j] += delta;
                    
                    // Update weight gradients and propagate error
                    for (size_t k = 0; k < weights[layer][j].size(); ++k) {
                        weight_gradients[layer][j][k] += delta * activations[layer][k];
                        if (layer > 0) {
                            next_errors[k] += delta * weights[layer][j][k];
                        }
                    }
                }
                
                // Set up for next layer
                errors = next_errors;
            }
        }
        
        // Apply gradients (divided by batch size)
        double scale = learning_rate / inputs.size();
        for (size_t i = 0; i < weights.size(); ++i) {
            for (size_t j = 0; j < weights[i].size(); ++j) {
                for (size_t k = 0; k < weights[i][j].size(); ++k) {
                    weights[i][j][k] -= scale * weight_gradients[i][j][k];
                }
                biases[i][j] -= scale * bias_gradients[i][j];
            }
        }
    }
}

// New method for batch training
void NeuralNetwork::train_batch(const std::vector<std::vector<double>>& inputs,
                               const std::vector<std::vector<double>>& targets) {
    if (inputs.size() != targets.size() || inputs.empty()) {
        std::cerr << "Error: Invalid batch data" << std::endl;
        return;
    }
    
    // Initialize gradients with zeros
    std::vector<std::vector<std::vector<double>>> weight_gradients(weights.size());
    std::vector<std::vector<double>> bias_gradients(biases.size());
    
    for (size_t i = 0; i < weights.size(); ++i) {
        weight_gradients[i].resize(weights[i].size());
        for (size_t j = 0; j < weights[i].size(); ++j) {
            weight_gradients[i][j].resize(weights[i][j].size(), 0.0);
        }
        bias_gradients[i].resize(biases[i].size(), 0.0);
    }
    
    // Process each training example
    for (size_t example = 0; example < inputs.size(); ++example) {
        const std::vector<double>& input = inputs[example];
        const std::vector<double>& target = targets[example];
        
        // Forward pass
        std::vector<std::vector<double>> activations = forward_pass(input);
        
        // Backward pass
        // Calculate output layer error
        std::vector<double> errors = activations.back();
        for (size_t i = 0; i < errors.size(); ++i) {
            errors[i] -= target[i];
        }
        
        // Backpropagate error and update gradients
        for (int layer = weights.size() - 1; layer >= 0; --layer) {
            std::vector<double> next_errors(layer_sizes[layer], 0.0);
            
            // For each neuron in this layer
            for (size_t j = 0; j < weights[layer].size(); ++j) {
                // Get activation derivative
                double delta;
                if (layer == weights.size() - 1) {
                    // Output layer
                    delta = errors[j];
                    if (output_activation != ActivationFunction::LINEAR) {
                        // For non-linear output, multiply by derivative
                        double out = activations[layer + 1][j];
                        delta *= activate_derivative(out, output_activation);
                    }
                } else {
                    // Hidden layer
                    delta = errors[j];
                    if (hidden_activation != ActivationFunction::LINEAR) {
                        // For non-linear hidden, multiply by derivative
                        double out = activations[layer + 1][j];
                        delta *= activate_derivative(out, hidden_activation);
                    }
                }
                
                // Update bias gradient
                bias_gradients[layer][j] += delta;
                
                // Update weight gradients and propagate error
                for (size_t k = 0; k < weights[layer][j].size(); ++k) {
                    weight_gradients[layer][j][k] += delta * activations[layer][k];
                    if (layer > 0) {
                        next_errors[k] += delta * weights[layer][j][k];
                    }
                }
            }
            
            // Set up for next layer
            errors = next_errors;
        }
    }
    
    // Apply gradients (divided by batch size)
    double scale = learning_rate / inputs.size();
    for (size_t i = 0; i < weights.size(); ++i) {
        for (size_t j = 0; j < weights[i].size(); ++j) {
            for (size_t k = 0; k < weights[i][j].size(); ++k) {
                weights[i][j][k] -= scale * weight_gradients[i][j][k];
            }
            biases[i][j] -= scale * bias_gradients[i][j];
        }
    }
}

// New method to copy weights from another network
void NeuralNetwork::copy_weights_from(const NeuralNetwork& other) {
    if (layer_sizes != other.layer_sizes) {
        std::cerr << "Error: Cannot copy weights from incompatible network" << std::endl;
        return;
    }
    
    weights = other.weights;
    biases = other.biases;
}

bool NeuralNetwork::save(const std::string& filename) const {
    std::ofstream file(filename, std::ios::binary);
    if (!file) {
        std::cerr << "Error: Could not open file for writing: " << filename << std::endl;
        return false;
    }
    
    // Save network architecture
    size_t num_layers = layer_sizes.size();
    file.write(reinterpret_cast<const char*>(&num_layers), sizeof(num_layers));
    
    for (size_t i = 0; i < num_layers; ++i) {
        file.write(reinterpret_cast<const char*>(&layer_sizes[i]), sizeof(layer_sizes[i]));
    }
    
    // Save learning rate
    file.write(reinterpret_cast<const char*>(&learning_rate), sizeof(learning_rate));
    
    // Save activation functions
    int hidden_act = static_cast<int>(hidden_activation);
    int output_act = static_cast<int>(output_activation);
    file.write(reinterpret_cast<const char*>(&hidden_act), sizeof(hidden_act));
    file.write(reinterpret_cast<const char*>(&output_act), sizeof(output_act));
    
    // Save weights and biases
    for (size_t i = 0; i < weights.size(); ++i) {
        for (size_t j = 0; j < weights[i].size(); ++j) {
            for (size_t k = 0; k < weights[i][j].size(); ++k) {
                file.write(reinterpret_cast<const char*>(&weights[i][j][k]), sizeof(weights[i][j][k]));
            }
            
            file.write(reinterpret_cast<const char*>(&biases[i][j]), sizeof(biases[i][j]));
        }
    }
    
    return true;
}

bool NeuralNetwork::load(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        std::cerr << "Error: Could not open file for reading: " << filename << std::endl;
        return false;
    }
    
    // Load network architecture
    size_t num_layers;
    file.read(reinterpret_cast<char*>(&num_layers), sizeof(num_layers));
    
    layer_sizes.resize(num_layers);
    for (size_t i = 0; i < num_layers; ++i) {
        file.read(reinterpret_cast<char*>(&layer_sizes[i]), sizeof(layer_sizes[i]));
    }
    
    // Load learning rate
    file.read(reinterpret_cast<char*>(&learning_rate), sizeof(learning_rate));
    
    // Load activation functions
    int hidden_act, output_act;
    file.read(reinterpret_cast<char*>(&hidden_act), sizeof(hidden_act));
    file.read(reinterpret_cast<char*>(&output_act), sizeof(output_act));
    hidden_activation = static_cast<ActivationFunction>(hidden_act);
    output_activation = static_cast<ActivationFunction>(output_act);
    
    // Initialize weights and biases structures
    weights.resize(num_layers - 1);
    biases.resize(num_layers - 1);
    
    for (size_t i = 0; i < num_layers - 1; ++i) {
        weights[i].resize(layer_sizes[i + 1]);
        biases[i].resize(layer_sizes[i + 1]);
        
        for (size_t j = 0; j < layer_sizes[i + 1]; ++j) {
            weights[i][j].resize(layer_sizes[i]);
            
            for (size_t k = 0; k < layer_sizes[i]; ++k) {
                file.read(reinterpret_cast<char*>(&weights[i][j][k]), sizeof(weights[i][j][k]));
            }
            
            file.read(reinterpret_cast<char*>(&biases[i][j]), sizeof(biases[i][j]));
        }
    }
    
    return true;
}
