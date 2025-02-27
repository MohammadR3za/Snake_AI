# Snake AI with Deep Q-Learning

This project implements the classic Snake game with three modes:
1. Manual play (keyboard control)
2. AI play (watch the trained agent play)
3. Training mode (train a new agent)

## Requirements

- C++17 compatible compiler
- CMake 3.10 or higher
- ncurses library (for Linux/macOS)

## Building the Project

```bash
mkdir build
cd build
cmake ..
make
```
## Running the Snake_AI
Manual Play
```bash
./snake_ai --mode=manual
```
AI Play (requires a trained model)
```bash
./snake_ai --mode=ai --load=snake_model_final.bin
```
Training Mode
```bash
./snake_ai --mode=train --episodes=10000
```
## Command Line Options

`--mode=<mode>:` Game mode (manual, ai, or train)

`--width=<width>:` Board width (default: 20)

`--height=<height>:` Board height (default: 15)

`--episodes=<num>:` Number of training episodes (default: 10000)

`--steps=<num>:` Max steps per episode (default: 1000)

`--load=<filename>:` Load model from file

`--save=<filename>:` Save model to file

`--config=<filename>:` Load configuration from file

`--help:` Display help message

## Controls (Manual Mode)
Arrow keys or WASD: Move snake

Q: Quit game

## Configuration File Format
```ini
exploration_rate=0.1
discount_factor=0.99
animation_delay_ms=100
visualize=true
```

## How the AI Works
This implementation uses Deep Q-Learning, a reinforcement learning technique that combines Q-learning with neural networks. The neural network takes the current game state as input and outputs Q-values for each possible action. The agent then selects the action with the highest Q-value.

The state representation includes:

Direction to food (normalized)
Danger in each direction
Current direction of the snake
Food location relative to snake head
Rewards:

Eating food: +10
Moving closer to food: +0.1
Moving away from food: -0.1
Dying: -10
Survival: +0.01 per step
The agent uses experience replay to improve learning stability, storing experiences in a replay buffer and randomly sampling from it during training.

## Project Structure
`main.cpp:` Entry point and command line parsing

`snake.h/cpp:` Snake game implementation and reinforcement learning logic

`nn.h/cpp:` Neural network implementation

`CMakeLists.txt:` CMake build configuration

```shell
## config.txt (example configuration file)

exploration_rate=0.1
discount_factor=0.99
animation_delay_ms=100
visualize=true
```

```markdown
This completes the implementation of the Snake AI using Deep Q-Learning. The project includes:

1. A complete Snake game with manual controls
2. A neural network implementation for deep Q-learning
3. Reinforcement learning logic with experience replay
4. Training and inference modes
5. Model saving/loading functionality
6. Configuration options
7. CMake build system

The code is structured to be readable and modular, making it easy to experiment with different neural network architectures, reward functions, and hyperparameters.
```
