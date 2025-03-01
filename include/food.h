
// food.h
#ifndef FOOD_H
#define FOOD_H

class Food {
public:
    Food(int width = 20, int height = 20);
    Food(unsigned x, int y);  // Added constructor for direct position setting
    
    int get_x() const { return x; }
    int get_y() const { return y; }
    void set_position(int new_x, int new_y);
    
private:
    int x;
    int y;
};

#endif // FOOD_H
