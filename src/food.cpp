
// food.cpp
#include "food.h"
#include <cstdlib>
#include <ctime>

Food::Food(int width, int height) {
    // Initialize random seed if not already done
    static bool seeded = false;
    if (!seeded) {
        srand(time(nullptr));
        seeded = true;
    }
    
    // Random position within the given dimensions
    x = rand() % width;
    y = rand() % height;
}

Food::Food(unsigned x, int y) : x(x), y(y) {}

void Food::set_position(int new_x, int new_y) {
    x = new_x;
    y = new_y;
}
