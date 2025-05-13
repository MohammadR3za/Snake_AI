#ifndef NN_H
#define NN_H

#include <random>
#include <vector>

class NeuralNetwork {
public:
  // Constructor
  NeuralNetwork(const std::vector<int> &topology);

  // Forward propagation
  std::vector<double> feedForward(const std::vector<double> &inputs);

  // Backpropagation training
  void backPropagate(const std::vector<double> &targets, double learningRate);

  // Get the predicted action
  int getAction(const std::vector<double> &gameState);

  // Q-learning update
  void updateQValues(const std::vector<double> &state, int action,
                     double reward, const std::vector<double> &newState,
                     double discount, double learningRate);

  // Save and load weights
  void saveWeights(const std::string &filename) const;
  bool loadWeights(const std::string &filename);

  double getError() { return lastError; };

private:
  // Topology (layers and neurons per layer)
  std::vector<int> topology;

  // Neuron layers
  std::vector<std::vector<double>> neurons; // [layer][neuron]

  // Weight connections
  std::vector<std::vector<std::vector<double>>>
      weights; // [layer][neuron][connection]

  // Biases
  std::vector<std::vector<double>> biases; // [layer][neuron]

  // Deltas for backpropagation
  std::vector<std::vector<double>> deltas; // [layer][neuron]

  // Random number generator
  std::mt19937 rng;

  // Last error for getFunction
  double lastError;

  // Helper methods
  double sigmoid(double x) const;
  double sigmoidDerivative(double x) const;
  double getTotalError(const std::vector<double> &targets) const;
};

#endif // NN_H
