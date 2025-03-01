
// neural_network.h
#ifndef NEURAL_NETWORK_H
#define NEURAL_NETWORK_H

#include <vector>
#include <string>

class NeuralNetwork {
public:
    NeuralNetwork(const std::vector<int>& topology);
    NeuralNetwork(const std::string& filename); // Constructor to load from file
    
    std::vector<double> feed_forward(const std::vector<double>& inputs) const;
    void set_weights(const std::vector<double>& weights);
    std::vector<double> get_weights() const;
    int get_weights_count() const;
    
    bool save_to_file(const std::string& filename) const;
    bool load_from_file(const std::string& filename);
    
private:
    std::vector<int> topology;
    std::vector<std::vector<std::vector<double>>> weights; // [layer][neuron][weight]
    std::vector<std::vector<double>> biases; // [layer][neuron]
    
    double activate(double x) const;
    double activate_derivative(double x) const;
};

#endif // NEURAL_NETWORK_H
