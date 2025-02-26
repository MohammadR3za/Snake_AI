#include "snake.h"
#include <iostream>
#include <sys/select.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <unistd.h>

#define WIDTH 30
#define HEIGHT 20

using namespace std;

typedef unsigned int uint;

int _kbhit() {
    static const int STDIN = 0;
    static bool initialized = false;
    if (!initialized) {
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


int main() {
    Snake snake(WIDTH, HEIGHT);
    cout << "Training the neural network..." << endl;
    snake.train(100, 1000); // Train with 100 games, 1000 epochs
    cout << "Training complete. Starting AI-controlled game..." << endl;
    snake.play();
    return 0;
}
