#include "point.hpp"
#include "platform/interface/input.hpp"

void translate(Point *p, Direction dir)
{
        switch (dir) {
        case Direction::UP:
                p->y -= 1;
                break;
        case Direction::DOWN:
                p->y += 1;
                break;
        case Direction::LEFT:
                p->x -= 1;
                break;
        case Direction::RIGHT:
                p->x += 1;
                break;
        default:
                // No translation for unknown direction
                break;
        }
}

void translate_within_bounds(Point *p, Direction dir, int rows, int cols)
{
        switch (dir) {
        case Direction::UP:
                if (p->y > 0)
                        p->y -= 1;
                break;
        case Direction::DOWN:
                if (p->y < rows - 1)
                        p->y += 1;
                break;
        case Direction::LEFT:
                if (p->x > 0)
                        p->x -= 1;
                break;
        case Direction::RIGHT:
                if (p->x < cols - 1)
                        p->x += 1;
                break;
        default:
                // No translation for unknown direction
                break;
        }
}

std::vector<Point> *get_neighbours_inside_grid(Point *point, int rows, int cols)
{
        std::vector<Point> *neighbours = new std::vector<Point>();
        neighbours->reserve(8);
        // Dereference for readability;
        Point p = *point;

        // We add adjacent neighbours if within grid
        if (p.y > 0)
                neighbours->push_back({.x = p.x, .y = p.y - 1});
        if (p.y < rows - 1)
                neighbours->push_back({.x = p.x, .y = p.y + 1});
        if (p.x > 0)
                neighbours->push_back({.x = p.x - 1, .y = p.y});
        if (p.x < cols - 1)
                neighbours->push_back({.x = p.x + 1, .y = p.y});

        // We add diagonal neighbours if within grid
        if (p.y > 0 && p.x > 0)
                neighbours->push_back({.x = p.x - 1, .y = p.y - 1});
        if (p.y < rows - 1 && p.x < cols - 1)
                neighbours->push_back({.x = p.x + 1, .y = p.y + 1});
        if (p.x > 0 && p.y < rows - 1)
                neighbours->push_back({.x = p.x - 1, .y = p.y + 1});
        if (p.x < cols - 1 && p.y > 0)
                neighbours->push_back({.x = p.x + 1, .y = p.y - 1});

        return neighbours;
}
