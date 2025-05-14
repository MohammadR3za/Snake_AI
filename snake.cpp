#include "snake.h"
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <ncurses.h>

SnakeGame::SnakeGame(int h, int w)
    : height(h), width(w), score(0), gameOver(false), direction(RIGHT) {
  // Initialize ncurses
  static bool ncursesInitialized = false;
  if (!ncursesInitialized) {
    initscr();
    cbreak();
    noecho();
    curs_set(0);          // Hide cursor
    timeout(100);         // Set input delay
    keypad(stdscr, TRUE); // Enable keyboard mapping
    ncursesInitialized = true;
  }

  // Create window
  win = newwin(height, width, 0, 0);
  box(win, 0, 0);
  wrefresh(win);

  // Initialize snake position at the center
  snake.clear(); // Clear any existing snake segments
  snake.push_back(std::make_pair(height / 2, width / 4));

  // Place initial food
  placeFood();

  // Initialize previous distance
  prevDistanceToFood = getDistanceToFood();
}

SnakeGame::~SnakeGame() {
  // Delete window but don't end ncurses
  // This allows multiple games to be created and destroyed
  delwin(win);
}

void SnakeGame::placeFood() {
  // Generate random position for food that's not on the snake
  int y, x;
  bool validPosition;

  do {
    validPosition = true;
    y = rand() % (height - 2) + 1;
    x = rand() % (width - 2) + 1;

    for (const auto &segment : snake) {
      if (segment.first == y && segment.second == x) {
        validPosition = false;
        break;
      }
    }
  } while (!validPosition);

  food = std::make_pair(y, x);
}

void SnakeGame::processInput() {
  int key = getch();

  switch (key) {
  case KEY_UP:
    if (direction != DOWN)
      direction = UP;
    break;
  case KEY_RIGHT:
    if (direction != LEFT)
      direction = RIGHT;
    break;
  case KEY_DOWN:
    if (direction != UP)
      direction = DOWN;
    break;
  case KEY_LEFT:
    if (direction != RIGHT)
      direction = LEFT;
    break;
  case 'q':
  case 'Q':
    gameOver = true;
    break;
  }
}

void SnakeGame::update() {
  // Get head position
  int headY = snake.front().first;
  int headX = snake.front().second;

  // Update previous distance for next reward calculation
  prevDistanceToFood = getDistanceToFood();

  // Calculate new head position based on direction
  switch (direction) {
  case UP:
    headY--;
    break;
  case RIGHT:
    headX++;
    break;
  case DOWN:
    headY++;
    break;
  case LEFT:
    headX--;
    break;
  }

  // Check for collisions

  // Wall collision
  if (headY <= 0 || headY >= height - 1 || headX <= 0 || headX >= width - 1) {
    gameOver = true;
    return;
  }

  // Self collision
  for (const auto &segment : snake) {
    if (headY == segment.first && headX == segment.second) {
      gameOver = true;
      return;
    }
  }

  // Move snake
  snake.push_front(std::make_pair(headY, headX));

  // Check if food is eaten
  if (headY == food.first && headX == food.second) {
    score++;
    placeFood();
  } else {
    // If food not eaten, remove tail
    snake.pop_back();
  }
}

void SnakeGame::render() {
  // Clear window
  wclear(win);

  // Draw border
  box(win, 0, 0);

  // Draw score
  mvwprintw(win, 0, 2, "Score: %d", score);

  // Draw snake
  for (const auto &segment : snake) {
    mvwaddch(win, segment.first, segment.second, 'O');
  }

  // Make head distinct
  mvwaddch(win, snake.front().first, snake.front().second, '@');

  // Draw food
  mvwaddch(win, food.first, food.second, '*');

  // Refresh window
  wrefresh(win);
}

bool SnakeGame::isGameOver() const { return gameOver; }

int SnakeGame::getScore() const { return score; }

void SnakeGame::run(bool aiMode) {
  while (!gameOver) {
    if (!aiMode) {
      processInput();
    }
    update();
    render();
    napms(10); // Sleep for smoother gameplay
  }

  // Game over message
  mvwprintw(win, height / 2, width / 2 - 5, "GAME OVER!");
  mvwprintw(win, height / 2 + 1, width / 2 - 7, "Final score: %d", score);
  mvwprintw(win, height / 2 + 2, width / 2 - 11, "Press any key to exit...");
  wrefresh(win);

  nodelay(stdscr, FALSE); // Wait for key press
  getch();
  nodelay(stdscr, TRUE); // Reset to non-blocking mode for next game
}

// AI-specific methods
std::vector<double> SnakeGame::getGameState() const {
  std::vector<double> state(8, 0.0);

  // Head position
  int headY = snake.front().first;
  int headX = snake.front().second;

  // 1-4: Danger straight, right, left
  switch (direction) {
  case UP:
    state[0] = (headY - 1 <= 0) ? 1.0 : 0.0;         // Wall up
    state[1] = (headX + 1 >= width - 1) ? 1.0 : 0.0; // Wall right
    state[2] = (headX - 1 <= 0) ? 1.0 : 0.0;         // Wall left
    break;
  case RIGHT:
    state[0] = (headX + 1 >= width - 1) ? 1.0 : 0.0;  // Wall right
    state[1] = (headY + 1 >= height - 1) ? 1.0 : 0.0; // Wall down
    state[2] = (headY - 1 <= 0) ? 1.0 : 0.0;          // Wall up
    break;
  case DOWN:
    state[0] = (headY + 1 >= height - 1) ? 1.0 : 0.0; // Wall down
    state[1] = (headX - 1 <= 0) ? 1.0 : 0.0;          // Wall left
    state[2] = (headX + 1 >= width - 1) ? 1.0 : 0.0;  // Wall right
    break;
  case LEFT:
    state[0] = (headX - 1 <= 0) ? 1.0 : 0.0;          // Wall left
    state[1] = (headY - 1 <= 0) ? 1.0 : 0.0;          // Wall up
    state[2] = (headY + 1 >= height - 1) ? 1.0 : 0.0; // Wall down
    break;
  }

  // Check for self-collision in those directions
  for (size_t i = 1; i < snake.size(); i++) {
    int segY = snake[i].first;
    int segX = snake[i].second;

    switch (direction) {
    case UP:
      if (headY - 1 == segY && headX == segX)
        state[0] = 1.0;
      if (headY == segY && headX + 1 == segX)
        state[1] = 1.0;
      if (headY == segY && headX - 1 == segX)
        state[2] = 1.0;
      break;
    case RIGHT:
      if (headY == segY && headX + 1 == segX)
        state[0] = 1.0;
      if (headY + 1 == segY && headX == segX)
        state[1] = 1.0;
      if (headY - 1 == segY && headX == segX)
        state[2] = 1.0;
      break;
    case DOWN:
      if (headY + 1 == segY && headX == segX)
        state[0] = 1.0;
      if (headY == segY && headX - 1 == segX)
        state[1] = 1.0;
      if (headY == segY && headX + 1 == segX)
        state[2] = 1.0;
      break;
    case LEFT:
      if (headY == segY && headX - 1 == segX)
        state[0] = 1.0;
      if (headY - 1 == segY && headX == segX)
        state[1] = 1.0;
      if (headY + 1 == segY && headX == segX)
        state[2] = 1.0;
      break;
    }
  }

  // 4-7: Direction
  state[3] = (direction == UP) ? 1.0 : 0.0;
  state[4] = (direction == RIGHT) ? 1.0 : 0.0;
  state[5] = (direction == DOWN) ? 1.0 : 0.0;
  state[6] = (direction == LEFT) ? 1.0 : 0.0;

  // 8: Food direction
  if (food.first < headY)
    state[7] = 1.0; // Food is above
  else if (food.first > headY)
    state[7] = 2.0; // Food is below
  else if (food.second < headX)
    state[7] = 3.0; // Food is left
  else if (food.second > headX)
    state[7] = 4.0; // Food is right

  return state;
}

void SnakeGame::setDirection(Direction dir) {
  // Prevent 180-degree turns
  if ((dir == UP && direction != DOWN) || (dir == RIGHT && direction != LEFT) ||
      (dir == DOWN && direction != UP) || (dir == LEFT && direction != RIGHT)) {
    direction = dir;
  }
}

double SnakeGame::getDistanceToFood() const {
  int headY = snake.front().first;
  int headX = snake.front().second;

  // Manhattan distance (more appropriate for grid-based movement)
  return std::abs(headY - food.first) + std::abs(headX - food.second);
}

double SnakeGame::calculateReward() const {
  // Base reward
  double reward = 0.0;

  // If game over, large negative reward
  if (gameOver) {
    return -1.0;
  }

  // If food eaten, large positive reward
  if (snake.size() > 1 && snake[0].first == food.first &&
      snake[0].second == food.second) {
    return 1.0;
  }

  // Small reward for getting closer to food
  double currentDistance = getDistanceToFood();
  if (currentDistance < prevDistanceToFood) {
    reward += 0.15;
  } else {
    reward -= 0.25;
  }
  return reward;
}

bool SnakeGame::willCollide(Direction dir) const {
  int headY = snake.front().first;
  int headX = snake.front().second;

  // Calculate potential new head position
  switch (dir) {
  case UP:
    headY--;
    break;
  case RIGHT:
    headX++;
    break;
  case DOWN:
    headY++;
    break;
  case LEFT:
    headX--;
    break;
  }

  // Wall collision
  if (headY <= 0 || headY >= height - 1 || headX <= 0 || headX >= width - 1) {
    return true;
  }

  // Self collision
  for (const auto &segment : snake) {
    if (headY == segment.first && headX == segment.second) {
      return true;
    }
  }

  return false;
}

// Add clean exit method to properly end ncurses when the program exits
void SnakeGame::cleanupNcurses() { endwin(); }
