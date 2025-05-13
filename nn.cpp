#include "nn.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <random>

NeuralNetwork::NeuralNetwork(const std::vector<int> &topology)
    : topology(topology) {
  // Initialize random number generator
  std::random_device rd;
  std::mt19937 rng(rd());
  std::uniform_real_distribution<double> dist(-1.0, 1.0);

  // Initialize layers
  neurons.resize(topology.size());
  deltas.resize(topology.size());

  for (size_t i = 0; i < topology.size(); ++i) {
    neurons[i].resize(topology[i], 0.0);
    deltas[i].resize(topology[i], 0.0);
  }

  // Initialize weights and biases
  weights.resize(topology.size() - 1);
  biases.resize(topology.size() - 1);

  for (size_t layer = 0; layer < weights.size(); ++layer) {
    weights[layer].resize(topology[layer + 1]);
    biases[layer].resize(topology[layer + 1]);

    for (size_t neuron = 0; neuron < topology[layer + 1]; ++neuron) {
      weights[layer][neuron].resize(topology[layer]);

      // Initialize random weights
      for (size_t w = 0; w < weights[layer][neuron].size(); ++w) {
        weights[layer][neuron][w] = dist(rng);
      }

      // Initialize random bias
      biases[layer][neuron] = dist(rng);
    }
  }
}

std::vector<double>
NeuralNetwork::feedForward(const std::vector<double> &inputs) {
  // Set input layer
  for (size_t i = 0; i < inputs.size(); ++i) {
    neurons[0][i] = inputs[i];
  }

  // Forward propagation
  for (size_t layer = 0; layer < weights.size(); ++layer) {
    for (size_t neuron = 0; neuron < topology[layer + 1]; ++neuron) {
      double sum = biases[layer][neuron];

      for (size_t input = 0; input < topology[layer]; ++input) {
        sum += neurons[layer][input] * weights[layer][neuron][input];
      }

      neurons[layer + 1][neuron] = sigmoid(sum);
    }
  }

  // Return output layer
  return neurons.back();
}

void NeuralNetwork::backPropagate(const std::vector<double> &targets,
                                  double learningRate) {
  // Calculate output layer deltas
  for (size_t i = 0; i < neurons.back().size(); ++i) {
    double output = neurons.back()[i];
    deltas.back()[i] = (targets[i] - output) * sigmoidDerivative(output);
  }

  // Calculate hidden layer deltas
  for (int layer = topology.size() - 2; layer > 0; --layer) {
    for (size_t neuron = 0; neuron < topology[layer]; ++neuron) {
      double error = 0.0;

      for (size_t nextNeuron = 0; nextNeuron < topology[layer + 1];
           ++nextNeuron) {
        error +=
            weights[layer][nextNeuron][neuron] * deltas[layer + 1][nextNeuron];
      }

      lastError = error;
      deltas[layer][neuron] = error * sigmoidDerivative(neurons[layer][neuron]);
    }
  }

  // Update weights and biases
  for (size_t layer = 0; layer < weights.size(); ++layer) {
    for (size_t neuron = 0; neuron < topology[layer + 1]; ++neuron) {
      for (size_t input = 0; input < topology[layer]; ++input) {
        weights[layer][neuron][input] +=
            learningRate * deltas[layer + 1][neuron] * neurons[layer][input];
      }

      biases[layer][neuron] += learningRate * deltas[layer + 1][neuron];
    }
  }
}

int NeuralNetwork::getAction(const std::vector<double> &gameState) {
  // Feed the game state through the network
  std::vector<double> outputs = feedForward(gameState);

  // Find the action with the highest Q-value
  return std::distance(outputs.begin(),
                       std::max_element(outputs.begin(), outputs.end()));
}

void NeuralNetwork::updateQValues(const std::vector<double> &state, int action,
                                  double reward,
                                  const std::vector<double> &newState,
                                  double discount, double learningRate) {
  // Current Q-values
  std::vector<double> currentQValues = feedForward(state);

  // Get max Q-value for the next state
  std::vector<double> nextQValues = feedForward(newState);
  double maxNextQ = *std::max_element(nextQValues.begin(), nextQValues.end());

  // Update the Q-value for the taken action using Q-learning formula
  // Q(s,a) = Q(s,a) + alpha * (reward + gamma * max(Q(s',a')) - Q(s,a))
  currentQValues[action] =
      currentQValues[action] +
      learningRate * (reward + discount * maxNextQ - currentQValues[action]);

  // Backpropagate to train the network
  backPropagate(currentQValues, learningRate);
}

double NeuralNetwork::sigmoid(double x) const { return 1.0 / (1.0 + exp(-x)); }

double NeuralNetwork::sigmoidDerivative(double x) const {
  return x * (1.0 - x);
}

double NeuralNetwork::getTotalError(const std::vector<double> &targets) const {
  double sum = 0.0;

  for (size_t i = 0; i < targets.size(); ++i) {
    double diff = targets[i] - neurons.back()[i];
    sum += diff * diff;
  }

  return sum / targets.size();
}

void NeuralNetwork::saveWeights(const std::string &filename) const {
  std::ofstream file(filename, std::ios::binary);

  if (!file) {
    std::cerr << "Error opening file for writing: " << filename << std::endl;
    return;
  }

  // Save topology
  size_t layerCount = topology.size();
  file.write(reinterpret_cast<const char *>(&layerCount), sizeof(layerCount));

  for (size_t i = 0; i < layerCount; ++i) {
    file.write(reinterpret_cast<const char *>(&topology[i]),
               sizeof(topology[i]));
  }

  // Save weights
  for (size_t layer = 0; layer < weights.size(); ++layer) {
    for (size_t neuron = 0; neuron < weights[layer].size(); ++neuron) {
      for (size_t w = 0; w < weights[layer][neuron].size(); ++w) {
        file.write(reinterpret_cast<const char *>(&weights[layer][neuron][w]),
                   sizeof(double));
      }
    }
  }

  // Save biases
  for (size_t layer = 0; layer < biases.size(); ++layer) {
    for (size_t neuron = 0; neuron < biases[layer].size(); ++neuron) {
      file.write(reinterpret_cast<const char *>(&biases[layer][neuron]),
                 sizeof(double));
    }
  }

  file.close();
}

bool NeuralNetwork::loadWeights(const std::string &filename) {
  std::ifstream file(filename, std::ios::binary);

  if (!file) {
    std::cerr << "Error opening file for reading: " << filename << std::endl;
    return false;
  }

  // Read topology
  size_t layerCount;
  file.read(reinterpret_cast<char *>(&layerCount), sizeof(layerCount));

  std::vector<int> loadedTopology(layerCount);
  for (size_t i = 0; i < layerCount; ++i) {
    file.read(reinterpret_cast<char *>(&loadedTopology[i]),
              sizeof(loadedTopology[i]));
  }

  // Check if topology matches
  if (loadedTopology != topology) {
    std::cerr << "Topology mismatch. Cannot load weights." << std::endl;
    return false;
  }

  // Read weights
  for (size_t layer = 0; layer < weights.size(); ++layer) {
    for (size_t neuron = 0; neuron < weights[layer].size(); ++neuron) {
      for (size_t w = 0; w < weights[layer][neuron].size(); ++w) {
        file.read(reinterpret_cast<char *>(&weights[layer][neuron][w]),
                  sizeof(double));
      }
    }
  }

  // Read biases
  for (size_t layer = 0; layer < biases.size(); ++layer) {
    for (size_t neuron = 0; neuron < biases[layer].size(); ++neuron) {
      file.read(reinterpret_cast<char *>(&biases[layer][neuron]),
                sizeof(double));
    }
  }

  file.close();
  return true;
}
