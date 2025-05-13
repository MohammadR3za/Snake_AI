#include "nn.h"
#include "snake.h"
#include <chrono>
#include <iostream>
#include <random>
#include <string>
#include <thread>

// Constants for RL
const double LEARNING_RATE = 0.05;
const double DISCOUNT_FACTOR = 0.95;
const double EXPLORATION_RATE_START = 1.0;
const double EXPLORATION_RATE_END = 0.01;
const int EXPLORATION_DECAY_STEPS = 10000;
const std::string WEIGHTS_FILE = "snake_ai_weights.bin";

// Function to train the neural network
void trainAI(int episodes) {
  // Create neural network with topology: input_size -> hidden_size ->
  // output_size Input: 8 neurons (see SnakeGame::getGameState()) Hidden: 16
  // neurons Output: 4 neurons (UP, RIGHT, DOWN, LEFT)
  NeuralNetwork nn({8, 32, 4});

  std::random_device rd;
  std::mt19937_64 rng(rd());
  std::uniform_real_distribution<double> fdist;
  std::uniform_int_distribution<int> idist(0, 3); // Fix the distribution range

  // Try to load existing weights
  bool weightsLoaded = nn.loadWeights(WEIGHTS_FILE);
  if (weightsLoaded) {
    std::cout << "Loaded existing weights from " << WEIGHTS_FILE << std::endl;
  }

  double exploration_rate = EXPLORATION_RATE_START;
  int totalSteps = 0;

  // Training loop
  for (int episode = 0; episode < episodes; ++episode) {
    // Initialize a NEW game environment for each episode
    SnakeGame game(20, 40);

    // Training stats
    int steps = 0;
    double totalReward = 0.0;

    // Print episode start
    clear(); // Clear the screen
    mvprintw(23, 0, "Training Episode %d of %d (Exploration Rate: %.2f)",
             episode + 1, episodes, exploration_rate);
    refresh();
    int lastScore = game.getScore();
    // Game loop for this episode
    while (!game.isGameOver()) {
      // Get current state
      std::vector<double> currentState = game.getGameState();

      // Choose action with epsilon-greedy strategy
      int action;
      if (fdist(rng) < exploration_rate) {
        // Explore: random action
        action = idist(rng);
      } else {
        // Exploit: best action according to Q-values
        action = nn.getAction(currentState);
      }

      // Convert action index to direction
      SnakeGame::Direction direction;
      switch (action) {
      case 0:
        direction = SnakeGame::UP;
        break;
      case 1:
        direction = SnakeGame::RIGHT;
        break;
      case 2:
        direction = SnakeGame::DOWN;
        break;
      case 3:
        direction = SnakeGame::LEFT;
        break;
      default:
        direction = SnakeGame::RIGHT;
      }

      // Take action
      game.setDirection(direction);
      game.update();

      // Calculate reward
      double reward = game.calculateReward();

      if (lastScore < game.getScore()) {
        lastScore = game.getScore();
      }

      if (steps % 50 == 0 && lastScore == game.getScore()) {
        reward += -100.0;
      }

      totalReward += reward;
      // Get new state
      std::vector<double> newState = game.getGameState();

      // Update Q-values
      nn.updateQValues(currentState, action, reward, newState, DISCOUNT_FACTOR,
                       LEARNING_RATE);

      // Render game for all training episodes (adjust frequency as needed)
      // if (totalSteps % 1 == 0) { // Render every 5 steps to avoid flickering
      //  game.render();
      //  mvprintw(21, 0, "Episode: %d/%d  Score: %d  Steps: %d", episode + 1,
      //           episodes, game.getScore(), steps);
      //  refresh();
      //  std::this_thread::sleep_for(std::chrono::milliseconds(2));
      //}

      steps++;
      totalSteps++;

      // Decay exploration rate
      exploration_rate =
          EXPLORATION_RATE_START +
          (EXPLORATION_RATE_END - EXPLORATION_RATE_START) *
              std::min(1.0, (double)totalSteps / EXPLORATION_DECAY_STEPS);

      // Check for user wanting to quit
      // int key = getch();
      // if (key == 'q' || key == 'Q') {
      //  mvprintw(10, 10, "Training interrupted by user!");
      //  refresh();
      //  std::this_thread::sleep_for(std::chrono::seconds(2));
      //  goto training_end; // Exit both loops
      //}
    }

    // Print episode statistics
    mvprintw(22, 0,
             "Episode %d complete: Steps = %d, Score = %d, Total Reward = %.2f "
             "Error: %lf",
             episode + 1, steps, game.getScore(), totalReward, nn.getError());
    refresh();
    std::this_thread::sleep_for(
        std::chrono::milliseconds(50)); // Pause between episodes

    // Save weights every 5 episodes (more frequent for small training runs)
    if (episode % 5 == 0 && episode > 0) {
      nn.saveWeights(WEIGHTS_FILE);
      mvprintw(3, 1, "Weights saved to %s", WEIGHTS_FILE.c_str());
      refresh();
    }
  }

training_end:
  std::cout << "Total steps across all episodes: " << totalSteps << std::endl;

  // Final save
  nn.saveWeights(WEIGHTS_FILE);
  std::cout << "Training completed. Final weights saved to " << WEIGHTS_FILE
            << std::endl;

  // Wait for key press before exiting
  mvprintw(5, 1, "Training complete. Press any key to continue...");
  refresh();
  nodelay(stdscr, FALSE); // Wait for key press
  getch();
  nodelay(stdscr, TRUE); // Reset to non-blocking mode
}

// Function to let AI play the game
void aiPlay() {
  // Create neural network with same topology
  NeuralNetwork nn({8, 32, 4});

  // Load trained weights
  if (!nn.loadWeights(WEIGHTS_FILE)) {
    std::cout << "Could not load weights. Please train the AI first."
              << std::endl;
    return;
  }

  std::cout << "AI is playing Snake. Press 'q' to quit." << std::endl;

  // Initialize game
  SnakeGame game(20, 40);

  // Game loop
  while (!game.isGameOver()) {
    // Get current state
    std::vector<double> state = game.getGameState();

    // Choose action
    int action = nn.getAction(state);

    // Convert action index to direction
    SnakeGame::Direction direction;
    switch (action) {
    case 0:
      direction = SnakeGame::UP;
      break;
    case 1:
      direction = SnakeGame::RIGHT;
      break;
    case 2:
      direction = SnakeGame::DOWN;
      break;
    case 3:
      direction = SnakeGame::LEFT;
      break;
    default:
      direction = SnakeGame::RIGHT;
    }

    // Take action
    game.setDirection(direction);

    // Check for user input to quit
    int key = getch();
    if (key == 'q' || key == 'Q') {
      break;
    }

    // Update and render the game
    game.update();
    game.render();

    // Sleep for better visualization
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  // Game over message will be shown by the game
}

int main(int argc, char *argv[]) {
  // Seed random number generator
  srand(time(nullptr));

  // Command line arguments
  if (argc > 1) {
    std::string arg = argv[1];

    if (arg == "--train" || arg == "-t") {
      int episodes = 1000;
      if (argc > 2) {
        episodes = std::stoi(argv[2]);
      }
      std::cout << "Training AI for " << episodes << " episodes..."
                << std::endl;
      trainAI(episodes);
      return 0;
    } else if (arg == "--ai" || arg == "-a") {
      aiPlay();
      return 0;
    }
  }

  // Default: Ask user what to do
  std::cout << "Snake Game with Neural Network" << std::endl;
  std::cout << "1. Play the game manually" << std::endl;
  std::cout << "2. Train the AI" << std::endl;
  std::cout << "3. Watch AI play" << std::endl;
  std::cout << "Enter your choice: ";

  int choice;
  std::cin >> choice;

  switch (choice) {
  case 1: {
    // Manual play
    SnakeGame game(20, 40);
    game.run(false);
    break;
  }
  case 2: {
    // Train AI
    std::cout << "How many episodes? ";
    int episodes;
    std::cin >> episodes;
    trainAI(episodes);
    break;
  }
  case 3:
    // AI play
    aiPlay();
    break;
  default:
    std::cout << "Invalid choice." << std::endl;
    break;
  }

  return 0;
}
