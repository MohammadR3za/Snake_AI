#ifndef NN_H
#define NN_H

#include <vector>
#include <string>

class NeuralNetwork {
public:
    enum class ActivationFunction {
        SIGMOID,
        TANH,
        RELU,
        LINEAR
    };

    NeuralNetwork() = default;
    NeuralNetwork(const std::vector<int>& layer_sizes, 
                  double learning_rate = 0.01,
                  ActivationFunction hidden_activation = ActivationFunction::RELU,
                  ActivationFunction output_activation = ActivationFunction::LINEAR);
    
    std::vector<double> predict(const std::vector<double>& input);
    void train(const std::vector<std::vector<double>>& inputs, 
               const std::vector<std::vector<double>>& targets,
               int epochs);
    
    // New methods for batch training and weight copying
    void train_batch(const std::vector<std::vector<double>>& inputs,
                    const std::vector<std::vector<double>>& targets);
    void copy_weights_from(const NeuralNetwork& other);
    
    bool save(const std::string& filename) const;
    bool load(const std::string& filename);

private:
    std::vector<int> layer_sizes;
    std::vector<std::vector<std::vector<double>>> weights;  // [layer][neuron][weight]
    std::vector<std::vector<double>> biases;               // [layer][neuron]
    double learning_rate;
    ActivationFunction hidden_activation;
    ActivationFunction output_activation;
    
    // Helper functions
    double activate(double x, ActivationFunction func) const;
    double activate_derivative(double x, ActivationFunction func) const;
    std::vector<std::vector<double>> forward_pass(const std::vector<double>& input);
};

#endif // NN_H
