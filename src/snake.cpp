#include "snake.h"
#include <iostream>
#include <unistd.h> // For usleep
#include <algorithm> // For std::max_element

using namespace std;

void moveCursor(Point point) {
    cout << "\033[" << point.y + 1 << ";" << point.x + 1 << "H";
}

Snake::Snake(uint width, uint height)
    : width(width), height(height), head(Point(width/4, height/4)), food(Point(width/2, height/2)),
      speed(100), direction(2), grow(false), score(0), highestScore(0),
      playedGames(0), isGameOver(false), isStartOfGame(true),
      nn({24, 16, 8, 4}, 0.1) // Initialize neural network: 24 inputs, 4 outputs
{
    area = new bool*[height];
    for (size_t i = 0; i < height; i++) {
        area[i] = new bool[width];
    }
    clearArea();
    segments.push_back(head);
    // Note: Game loop removed from constructor, moved to play()
}

Snake::~Snake() {
    for (size_t i = 0; i < height; i++) {
        delete[] area[i];
    }
    delete[] area;
}

std::vector<double> Snake::get_inputs() {
    std::vector<double> inputs(24, 0.0);
    int head_x = head.x;
    int head_y = head.y;

    // Directions: up, up-right, right, down-right, down, down-left, left, up-left
    int dx[] = {0, 1, 1, 1, 0, -1, -1, -1};
    int dy[] = {-1, -1, 0, 1, 1, 1, 0, -1};

    for (int i = 0; i < 8; ++i) {
        int next_x = head_x + dx[i];
        int next_y = head_y + dy[i];
        int index = i * 3;

        // Wall check
        if (next_x < 0 || next_x >= width || next_y < 0 || next_y >= height) {
            inputs[index] = 1.0;
        }
        // Food check
        else if (next_x == food.x && next_y == food.y) {
            inputs[index + 1] = 1.0;
        }
        // Body check
        else {
            for (const auto& segment : segments) {
                if (next_x == segment.x && next_y == segment.y) {
                    inputs[index + 2] = 1.0;
                    break;
                }
            }
        }
    }
    return inputs;
}

int Snake::choose_action() {
    int current_dir = direction;
    int head_x = head.x;
    int head_y = head.y;
    int food_x = food.x;
    int food_y = food.y;

    // Possible actions, excluding reverse
    std::vector<int> possible = {0, 1, 2, 3};
    possible.erase(std::remove_if(possible.begin(), possible.end(),
                                  [current_dir](int a) { return (a + 2) % 4 == current_dir; }),
                   possible.end());

    // Safety check lambda
    auto is_safe = [this](int x, int y) {
        if (x < 0 || x >= width || y < 0 || y >= height) return false;
        for (const auto& seg : segments) {
            if (seg.x == x && seg.y == y) return false;
        }
        return true;
    };

    std::vector<int> safe_actions;
    for (int action : possible) {
        int nx = head_x + (action == 2 ? 1 : action == 3 ? -1 : 0);
        int ny = head_y + (action == 0 ? -1 : action == 1 ? 1 : 0);
        if (is_safe(nx, ny)) safe_actions.push_back(action);
    }

    if (safe_actions.empty()) return possible[0]; // Will die, pick any

    // Prioritize moving toward food
    int curr_dist = std::abs(head_x - food_x) + std::abs(head_y - food_y);
    std::vector<int> towards_food;
    for (int action : safe_actions) {
        int nx = head_x + (action == 2 ? 1 : action == 3 ? -1 : 0);
        int ny = head_y + (action == 0 ? -1 : action == 1 ? 1 : 0);
        int new_dist = std::abs(nx - food_x) + std::abs(ny - food_y);
        if (new_dist < curr_dist) towards_food.push_back(action);
    }

    if (!towards_food.empty()) {
        return towards_food[rand() % towards_food.size()];
    }
    return safe_actions[rand() % safe_actions.size()];
}

std::vector<double> Snake::action_to_target(int action) {
    std::vector<double> target(4, 0.0);
    target[action] = 1.0;
    return target;
}

void Snake::collect_training_data(std::vector<std::vector<double>>& inputs,
                                 std::vector<std::vector<double>>& targets, int num_games) {
    for (int game = 0; game < num_games; ++game) {
        reset();
        while (true) {
            std::vector<double> input = get_inputs();
            int action = choose_action();
            direction = action;
            std::vector<double> target = action_to_target(action);
            inputs.push_back(input);
            targets.push_back(target);
            if (move()) break; // Collision ends the game
        }
    }
}

void Snake::train(int num_games, int epochs) {
    std::vector<std::vector<double>> inputs;
    std::vector<std::vector<double>> targets;
    collect_training_data(inputs, targets, num_games);
    nn.train(inputs, targets, epochs);
}

void Snake::play() {
    reset();
    drawArea();
    while (!isGameOver) {
        std::vector<double> input = get_inputs();
        std::vector<double> output = nn.predict(input);
        int action = std::distance(output.begin(), std::max_element(output.begin(), output.end()));
        // Prevent reversing
        if ((action + 2) % 4 != direction) {
            direction = action;
        }
        if (move()) {
            gameOver();
            break;
        }
        drawArea();
        usleep(150000); // Adjust speed as needed
    }
}

void Snake::drawArea() {
    if (isStartOfGame) {
        cout << "\033[2J\033[H" << std::flush; // Clear screen at start
        for (size_t h = 0; h < height; h++) {
            for (size_t w = 0; w < width; w++) {
                if (head.x == w && head.y == h) {
                    cout << "\033[32m■\033[0m";
                } else if (food.x == w && food.y == h) {
                    cout << "\033[31m●\033[0m";
                } else if (area[h][w]) {
                    cout << "\033[34m■\033[0m";
                } else {
                    cout << " ";
                }
            }
            cout << endl;
        }
        isStartOfGame = false;
    } else {
        if (!grow) {
            moveCursor(lastSegment);
            cout << ' ';
        }
        moveCursor(head);
        cout << "\033[32m■\033[0m";
        moveCursor(food);
        cout << "\033[31m●\033[0m";
        for (size_t i = 1; i < segments.size(); i++) {
            moveCursor(segments[i]);
            cout << "\033[35m#\033[0m";
        }
        moveCursor(Point(0, height + 1));
        cout << "Score: " << score << " | Highest Score: " << highestScore << endl;
    }
}

bool Snake::move() {
    switch (direction) {
        case 0: head.y--; break; // Up
        case 1: head.y++; break; // Down
        case 2: head.x++; break; // Right
        case 3: head.x--; break; // Left
    }

    if (hasCollision()) {
        return true; // Collision occurred
    }

    if (head == food) {
        grow = true;
        score++;
        if (score > highestScore) highestScore = score;
        createFood();
    }

    if (!grow) {
        lastSegment = segments.back();
        segments.pop_back();
    } else {
        grow = false;
    }
    segments.insert(segments.begin(), head);
    return false; // No collision
}

void Snake::clearArea() {
    for (size_t h = 0; h < height; h++) {
        for (size_t w = 0; w < width; w++) {
            area[h][w] = (h == 0 || h == height - 1 || w == 0 || w == width - 1);
        }
    }
}

void Snake::createFood() {
    do {
        food = Point(std::max(1, rand() % ((int)width - 2)), std::max(1, rand() % ((int)height - 2)));
    } while (std::find(segments.begin(), segments.end(), food) != segments.end());
}

void Snake::reset() {
    score = 0;
    head = Point(width / 4, height / 4);
    food = Point(width / 2, height / 2);
    direction = 2; // Start moving right
    clearArea();
    segments.clear();
    segments.push_back(head);
    isGameOver = false;
    isStartOfGame = true;
}

void Snake::gameOver() {
    cout << "\033[2J\033[H" << std::flush;
    cout << "\033[31mGame Over! | Score: " << score << "\033[0m" << endl;
    isGameOver = true;
}

bool Snake::hasCollision() {
    if (head.x <= 0 || head.x >= width - 1 || head.y <= 0 || head.y >= height - 1) {
        return true;
    }
    for (size_t i = 1; i < segments.size(); i++) {
        if (segments[i] == head) return true;
    }
    return false;
}
