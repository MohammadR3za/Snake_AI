#include "nn.h"
#include <random>
#include <iostream>

NeuralNetwork::NeuralNetwork(const std::vector<int>& layer_sizes, double lr)
    : layer_sizes(layer_sizes), learning_rate(lr) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(-1.0, 1.0);

    // Initialize weights and biases
    for (size_t i = 1; i < layer_sizes.size(); ++i) {
        std::vector<std::vector<double>> layer_weights(layer_sizes[i - 1],
                                                       std::vector<double>(layer_sizes[i]));
        for (int j = 0; j < layer_sizes[i - 1]; ++j) {
            for (int k = 0; k < layer_sizes[i]; ++k) {
                layer_weights[j][k] = dis(gen) * 0.1; // Small initial weights
            }
        }
        weights.push_back(layer_weights);

        std::vector<double> layer_biases(layer_sizes[i], 0.0);
        for (int k = 0; k < layer_sizes[i]; ++k) {
            layer_biases[k] = dis(gen) * 0.1;
        }
        biases.push_back(layer_biases);
    }
}

std::vector<double> NeuralNetwork::forward(const std::vector<double>& input) const {
    std::vector<std::vector<double>> layer_outputs = {input};

    for (size_t i = 0; i < weights.size(); ++i) {
        const auto& prev_output = layer_outputs.back();
        std::vector<double> layer_output(biases[i].size(), 0.0);
        for (size_t j = 0; j < layer_output.size(); ++j) {
            for (size_t k = 0; k < prev_output.size(); ++k) {
                layer_output[j] += prev_output[k] * weights[i][k][j];
            }
            layer_output[j] += biases[i][j];
            layer_output[j] = sigmoid(layer_output[j]);
        }
        layer_outputs.push_back(layer_output);
    }

    return layer_outputs.back();
}

void NeuralNetwork::train(const std::vector<std::vector<double>>& inputs,
                          const std::vector<std::vector<double>>& targets, int epochs) {
    for (int epoch = 0; epoch < epochs; ++epoch) {
        double total_error = 0.0;
        for (size_t k = 0; k < inputs.size(); ++k) {
            // Forward pass
            std::vector<std::vector<double>> layer_outputs = {inputs[k]};
            for (size_t i = 0; i < weights.size(); ++i) {
                const auto& prev_output = layer_outputs.back();
                std::vector<double> layer_output(biases[i].size(), 0.0);
                for (size_t j = 0; j < layer_output.size(); ++j) {
                    for (size_t m = 0; m < prev_output.size(); ++m) {
                        layer_output[j] += prev_output[m] * weights[i][m][j];
                    }
                    layer_output[j] += biases[i][j];
                    layer_output[j] = sigmoid(layer_output[j]);
                }
                layer_outputs.push_back(layer_output);
            }

            // Compute errors for output layer
            std::vector<std::vector<double>> errors(weights.size());
            errors.back().resize(layer_sizes.back());
            for (size_t j = 0; j < layer_sizes.back(); ++j) {
                double output = layer_outputs.back()[j];
                errors.back()[j] = (targets[k][j] - output) * sigmoid_derivative(output);
                total_error += (targets[k][j] - output) * (targets[k][j] - output);
            }

            // Backpropagate errors
            for (int i = weights.size() - 2; i >= 0; --i) {
                errors[i].resize(layer_sizes[i + 1]);
                for (size_t j = 0; j < layer_sizes[i + 1]; ++j) {
                    errors[i][j] = 0.0;
                    for (size_t m = 0; m < layer_sizes[i + 2]; ++m) {
                        errors[i][j] += errors[i + 1][m] * weights[i + 1][j][m];
                    }
                    errors[i][j] *= sigmoid_derivative(layer_outputs[i + 1][j]);
                }
            }

            // Update weights and biases
            for (size_t i = 0; i < weights.size(); ++i) {
                const auto& prev_output = (i == 0) ? inputs[k] : layer_outputs[i];
                for (size_t j = 0; j < weights[i].size(); ++j) {
                    for (size_t m = 0; m < weights[i][j].size(); ++m) {
                        weights[i][j][m] += learning_rate * errors[i][m] * prev_output[j];
                    }
                }
                for (size_t m = 0; m < biases[i].size(); ++m) {
                    biases[i][m] += learning_rate * errors[i][m];
                }
            }
        }
        if (epoch % 100 == 0) {
            std::cout << "Epoch " << epoch << ", Error: " << total_error / inputs.size() << std::endl;
        }
    }
}

std::vector<double> NeuralNetwork::predict(const std::vector<double>& input) const {
    return forward(input);
}
