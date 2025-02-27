#include "nn.h"
#include <iostream>

int main() {
    // Create a neural network with 2 inputs, 2 hidden neurons, 1 output
    NeuralNetwork nn(2, 2, 1, 0.5);

    // Training data for XOR
    std::vector<std::vector<double>> inputs = {
        {0.0, 0.0},
        {0.0, 1.0},
        {1.0, 0.0},
        {1.0, 1.0}
    };
    std::vector<std::vector<double>> targets = {
        {0.0},
        {1.0},
        {1.0},
        {0.0}
    };

    // Train the network
    nn.train(inputs, targets, 10000);

    // Test predictions
    for (const auto& input : inputs) {
        std::vector<double> output = nn.predict(input);
        std::cout << "Input: " << input[0] << ", " << input[1]
                  << " Output: " << output[0] << std::endl;
    }

    return 0;
}
