#ifndef SNAKE_H
#define SNAKE_H

#include <deque>
#include <ncurses.h>
#include <utility>
#include <vector>

class SnakeGame {
public:
  // Directions
  enum Direction { UP = 0, RIGHT = 1, DOWN = 2, LEFT = 3 };

  // Constructor and destructor
  SnakeGame(int h, int w);
  ~SnakeGame();

  // Core gameplay methods
  void processInput();
  void update();
  void render();
  bool isGameOver() const;
  int getScore() const;
  void run(bool aiMode = false);

  // AI-specific methods
  std::vector<double> getGameState() const;
  void setDirection(Direction dir);
  double calculateReward() const;

  // Clean up ncurses (call at program exit)
  static void cleanupNcurses();

private:
  int height, width;
  int score;
  bool gameOver;

  // Snake body as a deque of coordinates
  std::deque<std::pair<int, int>> snake;

  // Food position
  std::pair<int, int> food;

  // Direction: 0=up, 1=right, 2=down, 3=left
  Direction direction;

  // Previous distance to food (for reward calculation)
  double prevDistanceToFood;

  // Terminal window
  WINDOW *win;

  // Internal methods
  void placeFood();
  double getDistanceToFood() const;
  bool willCollide(Direction dir) const;
};

#endif // SNAKE_H
