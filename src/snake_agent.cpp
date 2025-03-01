
// snake_agent.cpp
#include "snake_agent.h"
#include <algorithm>

SnakeAgent::SnakeAgent() : brain(std::vector<int>{24, 16, 4}) {
    // Default constructor with standard topology
}

SnakeAgent::SnakeAgent(const NeuralNetwork& brain) : brain(brain) {
    // Constructor taking a neural network
}

SnakeAgent::SnakeAgent(const std::vector<int>& topology) : brain(topology) {
    // Constructor with custom topology
}

Direction SnakeAgent::make_decision(const GameState& state) const {
    std::vector<double> state_representation = state.get_vision_data();
    return get_move(state_representation);
}

Direction SnakeAgent::get_move(const std::vector<double>& state) const {
    // Feed state to neural network
    std::vector<double> outputs = brain.feed_forward(state);
    
    // Find the highest output (argmax)
    int max_index = 0;
    double max_value = outputs[0];
    
    for (size_t i = 1; i < outputs.size(); i++) {
        if (outputs[i] > max_value) {
            max_value = outputs[i];
            max_index = i;
        }
    }
    
    // Convert index to direction
    return int_to_direction(max_index);
}

void SnakeAgent::set_brain(const NeuralNetwork& new_brain) {
    brain = new_brain;
}

NeuralNetwork SnakeAgent::get_brain() const {
    return brain;
}

bool SnakeAgent::save_to_file(const std::string& filename) const {
    return brain.save_to_file(filename);
}

bool SnakeAgent::load_from_file(const std::string& filename) {
    if(!brain.load_from_file(filename))
      return false;
    return true;
}
