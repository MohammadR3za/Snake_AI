
// snake_agent.h
#ifndef SNAKE_AGENT_H
#define SNAKE_AGENT_H

#include "neural_network.h"
#include "game_state.h"
#include "direction.h"
#include <vector>
#include <string>

class SnakeAgent {
public:
    SnakeAgent();
    SnakeAgent(const NeuralNetwork& brain);  // Constructor taking a neural network
    SnakeAgent(const std::vector<int>& topology);
    
    Direction make_decision(const GameState& state) const;
    Direction get_move(const std::vector<double>& state) const;  // Added method for compatibility
    
    void set_brain(const NeuralNetwork& brain);
    NeuralNetwork get_brain() const;
    
    bool save_to_file(const std::string& filename) const;
    bool load_from_file(const std::string& filename);  // Static method for loading
    
private:
    NeuralNetwork brain;
};

#endif // SNAKE_AGENT_H
