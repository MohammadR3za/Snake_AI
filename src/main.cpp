#include <iostream>
#include <vector>
#include <stdio.h>
#include <sys/select.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <algorithm>

#define WIDTH 30
#define HEIGHT 20

using namespace std;

typedef unsigned int uint;
typedef unsigned long uint16;
 
int _kbhit() {
  static const int STDIN = 0;
  static bool initialized = false;
  if (! initialized) {
    // Use termios to turn off line buffering
    termios term;
    tcgetattr(STDIN, &term);
    term.c_lflag &= ~ICANON;
    tcsetattr(STDIN, TCSANOW, &term);
    setbuf(stdin, NULL);
    initialized = true;
  }
  int bytesWaiting;
  ioctl(STDIN, FIONREAD, &bytesWaiting);
  return bytesWaiting;
}

class Point {
public:
  Point () {};
  Point(int nx,int ny) : x(nx), y(ny) {

  }
  int x = 0;
  int y = 0;
  bool operator==(Point rvalue)
  {
    return x==rvalue.x && y==rvalue.y;
  }
  void operator=(Point rvalue)
  {
    x = rvalue.x;
    y = rvalue.y;
  }
};

class Snake {
public:
  Snake(uint width, uint height);
  ~Snake() {};
  void drawArea();
  void move();
  void createFood();
  void reset();
  bool hasCollision();
  void gameOver();
private:
  bool **area;
  vector<Point> segments;
  Point lastSegment;
  uint width,height;
  Point head, food;
  uint speed, direction; // dirextion 0:up, 1:down, 2:roght, left:3
  bool grow;
  uint score, highestScore, playedGames;
  bool isGameOver;
  bool isStartOfGame;
  void clearArea();
};

void printTitle(uint epuchs, double loss, unsigned highestScore);
void moveCursor(Point point);

int main (int argc, char *argv[]) {
  Snake snake(WIDTH, HEIGHT);
  return 0;
}


void printTitle(uint epuchs, double loss, unsigned highestScore)
{
  print("{0} | Excuted {1} epuchs | Loss: {2} | Highest Score: {3}\n", "Snake game with AI", epuchs, loss, highestScore);
}

void moveCursor(Point point)
{
   cout << "\033[" << point.y+1 << ";" << point.x+1 << "H";
}

Snake::Snake(uint width, uint height)
: width(width), height(height), head(Point(width/4,height/4)), food(Point(width/2,height/2)),
  speed(100), direction(2), grow(false), score(0), highestScore(0),
  playedGames(0), isGameOver(false), isStartOfGame(true)
{
  area = new bool*[height];
  cout << "\033[2J\033[H" << std::flush;
  for (size_t i = 0; i < height; i++) {
    area[i] = new bool[width];
  }
  clearArea();
  segments.push_back(head);
  drawArea();
  while (!isGameOver) {
    if(_kbhit()) {
      char pressed;
      if(read(STDIN_FILENO, &pressed, 1)) {
        switch (pressed) {
          case 'w': if(direction != 1) direction = 0; break;
          case 's': if(direction != 0) direction = 1; break;
          case 'd': if(direction != 3) direction = 2; break;
          case 'a': if(direction != 2) direction = 3; break;
          default:
        
            break;
        }
      }
    }
    move();
    if (direction == 1 || direction == 0) {
      usleep(200000);
    } else
      usleep(150000);
  }
}

void Snake::drawArea()
{
  //cout << "\033[2J\033[H" << std::flush;
  if(isStartOfGame) {
    for (size_t h = 0; h < height; h++) {
      for (size_t w = 0; w < width; w++) {
        if (head.x == w && head.y == h) {
          print("\033[32m{0}\033[0m", "■");
        } else if (food.x == w && food.y == h) {
          print("\033[31m{0}\033[0m", "●");
        } else if (area[h][w]) {
          print("\033[34m{0}\033[0m", "■");
        } else {
          print(" ");
        }
      }
      print("\n");
    }
    isStartOfGame = false;
  }
  if (!grow) {
    moveCursor(lastSegment);
    cout << ' ';
  } else grow = false;
  moveCursor(head);
  print("\033[32m{0}\033[0m", "■");

  moveCursor(food);
  print("\033[31m{0}\033[0m", "●");

  for(size_t i = 1; i < segments.size(); i++) {
    moveCursor(segments[i]);
    print("\033[35m{0}\033[0m", "#");
  }
  moveCursor(Point(0, 26));
  printTitle(head.x, segments[0].x, highestScore);
}

void Snake::move()
{
  switch (direction) {
    case 0: head.y--; break;
    case 1: head.y++; break;
    case 2: head.x++; break;
    case 3: head.x--; break;
    default:
      print("\033[31m{0}\033[0m", "Wrong direction to move!");
      return;
      break;
  }

  if (hasCollision()) {
    gameOver();
    return;
  }

  if (head == food) {
    grow = true;
    score++;
    if (score > highestScore) {
      highestScore = score;
    }
    createFood();
  }

  if(!grow) {
    lastSegment = segments.back();
    segments.pop_back();
  }

  segments.insert(segments.begin(),head);
  drawArea();
}

void Snake::clearArea()
{
  for (size_t h = 0; h < height; h++) {
    for (size_t w = 0; w < width; w++) {
      if (h == 0 || h == height-1 || w == 0 || w == width-1) {
        area[h][w] = true;
      } else {
        area[h][w] = false;
      }
    }
  }
}

void Snake::createFood()
{
  moveCursor(food);
  cout << ' ';
  food = Point(std::max(1,rand()%((int)width-2)),std::max(1,rand()%((int)height-2)));
  bool reCreate = false;
  for (size_t i = 0; i < segments.size(); i++) {
    if (food == segments[i]) {
      reCreate = true;
    }
  }
  if (reCreate) {
    createFood();
  }
}

void Snake::reset()
{
  score = 0;
  head = Point(width/4,height/4);
  food = Point(width/2, height/2);
  clearArea();
  segments.clear();
  segments.insert(segments.end(), head);
}

void Snake::gameOver()
{
  cout << "\033[2J\033[H" << std::flush;
  print("\033[31m{0} | {1}\033[0m", "Game Over! the Score is",score);
  isGameOver = true;
}

bool Snake::hasCollision()
{
  if (head.x == width-1 || head.x == 0
       || head.y == height-1 || head.y == 0) {
    return true;
  }
  for (size_t i = 1; i < segments.size(); i++) {
    if(segments[i] == head)
      return true;
  }
  return false;
}
