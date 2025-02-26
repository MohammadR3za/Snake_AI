#ifndef NN_H
#define NN_H

#include <vector>
#include <cmath>

class NeuralNetwork {
private:
    std::vector<int> layer_sizes; // e.g., {24, 16, 8, 4}
    std::vector<std::vector<std::vector<double>>> weights; // Weights between layers
    std::vector<std::vector<double>> biases; // Biases for each layer (except input)
    double learning_rate;

    double sigmoid(double x) const {
        return 1.0 / (1.0 + std::exp(-x));
    }

    double sigmoid_derivative(double x) const {
        return x * (1.0 - x); // x is the sigmoid output
    }

public:
    NeuralNetwork(const std::vector<int>& layer_sizes, double lr = 0.1);
    std::vector<double> forward(const std::vector<double>& input) const;
    void train(const std::vector<std::vector<double>>& inputs,
               const std::vector<std::vector<double>>& targets, int epochs);
    std::vector<double> predict(const std::vector<double>& input) const;
};

#endif // NN_H
