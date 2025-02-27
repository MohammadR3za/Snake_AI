#ifndef COMMON_H
#define COMMON_H

#include <random>
#include <memory>
#include <vector>
#include <algorithm>
#include <iostream>

// Common utility functions
namespace common {

// Generate a random integer in range [min, max]
inline int random_int(int min, int max) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dist(min, max);
    return dist(gen);
}

// Generate a random double in range [min, max]
inline double random_double(double min, double max) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_real_distribution<double> dist(min, max);
    return dist(gen);
}

// Generate a random double from normal distribution
inline double random_normal(double mean, double stddev) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::normal_distribution<double> dist(mean, stddev);
    return dist(gen);
}

// Sigmoid activation function
inline double sigmoid(double x) {
    return 1.0 / (1.0 + std::exp(-x));
}

// ReLU activation function
inline double relu(double x) {
    return std::max(0.0, x);
}

// Leaky ReLU activation function
inline double leaky_relu(double x, double alpha = 0.01) {
    return x > 0.0 ? x : alpha * x;
}

// Tanh activation function
inline double tanh_activation(double x) {
    return std::tanh(x);
}

// Calculate Euclidean distance between two points
template<typename T>
inline double euclidean_distance(T x1, T y1, T x2, T y2) {
    return std::sqrt(std::pow(x2 - x1, 2) + std::pow(y2 - y1, 2));
}

// Calculate Manhattan distance between two points
template<typename T>
inline T manhattan_distance(T x1, T y1, T x2, T y2) {
    return std::abs(x2 - x1) + std::abs(y2 - y1);
}

}  // namespace common

#endif // COMMON_H
