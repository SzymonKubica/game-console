#pragma once
#include "platform/interface/input.hpp"
#include <vector>
typedef struct Point {
        int x;
        int y;
} Point;

void translate(Point *p, Direction dir);
void translate_within_bounds(Point *p, Direction dir, int rows, int cols);
std::vector<Point> *get_neighbours_inside_grid(Point *point, int rows, int cols);

bool is_adjacent(Point *p1, Point *p2);
