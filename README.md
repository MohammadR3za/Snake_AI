# Snake AI with Neural Networks and Genetic Algorithm

This project implements an AI for the classic Snake game using neural networks trained with a genetic algorithm.

## Overview

The Snake AI uses a neural network to control the snake's movement. The network is trained using a genetic algorithm that evolves a population of neural networks over multiple generations to maximize the snake's performance.

## Key Features

- Neural network-controlled snake with configurable architecture
- Genetic algorithm with advanced selection, crossover, and mutation strategies
- Improved fitness function that rewards exploration and efficiency
- Visualization of the best-performing AI
- Training resumption from saved networks
- Configurable training parameters

## Building the Project

### Prerequisites

- C++17 compatible compiler
- SDL2 library (for visualization)
- SDL2_ttf library (for text rendering)
- CMake (3.10 or higher)

### Build Instructions

1. Clone the repository:
   ```bash
     git clone https://github.com/yourusername/snake-ai.git
     cd snake-ai
   ```
   
2. Create a build directory
   ```bash
   mkdir build
   cd build
   ```

3. Configure and build:
   ```bash
   cmake
   make
   ```

## Usage

### Training the AI

To train a new neural network:
   ```bash
  ./snake_ai --train
  ```


With custom parameters:
  ```bash
  ./snake_ai --train --population 200 --generations 300 --mutation 0.2 --crossover 0.7
  ```

To resume training from a saved network:
  ```bash
  ./snake_ai --train --resume --load best_network.bin
  ```

### Running a Trained AI

To watch a trained AI play:
  ```bash
  ./snake_ai --load best_network.bin
  ```

### Command Line Options

- `--help`: Show help message
- `--train`: Train the neural network
- `--load <file>`: Load a trained neural network from file
- `--resume`: Resume training from a loaded network
- `--headless`: Run without graphical output (training only)
- `--visualize-training`: Show the best individual during training
- `--grid-width <n>`: Set grid width (default: 20)
- `--grid-height <n>`: Set grid height (default: 20)
- `--window-width <n>`: Set window width (default: 800)
- `--window-height <n>`: Set window height (default: 800)
- `--delay <ms>`: Set frame delay in milliseconds (default: 50)
- `--population <n>`: Set population size (default: 200)
- `--mutation <rate>`: Set mutation rate (default: 0.2)
- `--crossover <rate>`: Set crossover rate (default: 0.7)
- `--elite <n>`: Set number of elite individuals (default: 10)
- `--generations <n>`: Set number of generations (default: 200)
- `--games <n>`: Set games per individual (default: 5)
- `--output-dir <dir>`: Set output directory for saved networks
- `--architecture <sizes>`: Set network architecture (comma-separated layer sizes)

## Neural Network Architecture

The default neural network architecture is:
- Input layer: 16 neurons (snake position, food position, direction to food, danger detection, current direction)
- Hidden layer 1: 32 neurons
- Hidden layer 2: 16 neurons
- Output layer: 4 neurons (up, right, down, left)

## Genetic Algorithm Parameters

- Population size: 200 individuals
- Mutation rate: 0.2 (20% chance per weight/bias)
- Crossover rate: 0.7 (70% chance of crossover between parents)
- Elite count: 10 (top 10 individuals preserved unchanged)
- Generations: 200 (training iterations)

## Improvements

The key improvements in this version include:
1. Enhanced fitness function that rewards exploration and efficiency
2. Adaptive mutation rates based on population diversity
3. Multiple crossover methods for better genetic diversity
4. More sophisticated neural network inputs for better decision-making
5. Larger neural network architecture for more complex behavior
6. Improved training parameters for faster convergence

## License

This project is licensed under the MIT License - see the LICENSE file for details.
