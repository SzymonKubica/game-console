#include "geometry.hpp"
#include "stdlib.h"
#include "../platform/interface/input.hpp"

#include "maths_utils.hpp"
#include <cmath>
#include <optional>

/* === IntPoint Implementations === */

/**
 * Component-wise add two IntPoints together and returns the result.
 */
IntPoint operator+(IntPoint first, IntPoint second)
{
        return {first.x + second.x, first.y + second.y};
}

IntPoint IntPoint::operator*(int scalar) const
{
        return {scalar * x, scalar * y};
}

bool operator==(const IntPoint &first, const IntPoint &second)
{
        return first.x == second.x && first.y == second.y;
}

void translate(IntPoint &p, Direction dir)
{
        switch (dir) {
        case Direction::UP:
                p.y -= 1;
                break;
        case Direction::DOWN:
                p.y += 1;
                break;
        case Direction::LEFT:
                p.x -= 1;
                break;
        case Direction::RIGHT:
                p.x += 1;
                break;
        default:
                // No translation for unknown direction
                break;
        }
}

IntPoint translate_pure(const IntPoint &p, Direction dir)
{
        switch (dir) {
        case Direction::UP:
                return {p.x, p.y - 1};
        case Direction::DOWN:
                return {p.x, p.y + 1};
        case Direction::LEFT:
                return {p.x - 1, p.y};
        case Direction::RIGHT:
                return {p.x + 1, p.y};
        default:
                // No translation for unknown direction
                return p;
        }
}

std::optional<Direction> determine_displacement_direction(IntPoint &reference,
                                                          IntPoint &target)
{

        bool same_column = reference.x == target.x;
        bool same_row = reference.y == target.y;
        if (same_column) {
                if (reference.y - 1 == target.y)
                        return Direction::UP;
                if (reference.y + 1 == target.y)
                        return Direction::DOWN;
        }
        if (same_row) {
                if (reference.x + 1 == target.x)
                        return Direction::RIGHT;
                if (reference.x - 1 == target.x)
                        return Direction::LEFT;
        }
        return std::nullopt;
}

void translate_within_bounds(IntPoint &p, Direction dir, int rows, int cols)
{
        switch (dir) {
        case Direction::UP:
                if (p.y > 0)
                        p.y -= 1;
                break;
        case Direction::DOWN:
                if (p.y < rows - 1)
                        p.y += 1;
                break;
        case Direction::LEFT:
                if (p.x > 0)
                        p.x -= 1;
                break;
        case Direction::RIGHT:
                if (p.x < cols - 1)
                        p.x += 1;
                break;
        default:
                // No translation for unknown direction
                break;
        }
}

void translate_toroidal_array(IntPoint &p, Direction dir, int rows, int cols)
{
        switch (dir) {
        case Direction::UP:
                p.y = mathematical_modulo(p.y - 1, rows);
                break;
        case Direction::DOWN:
                p.y = mathematical_modulo(p.y + 1, rows);
                break;
        case Direction::LEFT:
                p.x = mathematical_modulo(p.x - 1, cols);
                break;
        case Direction::RIGHT:
                p.x = mathematical_modulo(p.x + 1, cols);
                break;
        default:
                // No translation for unknown direction
                break;
        }
}

std::vector<IntPoint> get_neighbours_inside_grid(const IntPoint &point,
                                                 int rows, int cols)
{
        std::vector<IntPoint> neighbours;
        // alias for readability;
        auto &p = point;

        // We add adjacent neighbours if within grid
        if (p.y > 0)
                neighbours.push_back({.x = p.x, .y = p.y - 1});
        if (p.y < rows - 1)
                neighbours.push_back({.x = p.x, .y = p.y + 1});
        if (p.x > 0)
                neighbours.push_back({.x = p.x - 1, .y = p.y});
        if (p.x < cols - 1)
                neighbours.push_back({.x = p.x + 1, .y = p.y});

        // We add diagonal neighbours if within grid
        if (p.y > 0 && p.x > 0)
                neighbours.push_back({.x = p.x - 1, .y = p.y - 1});
        if (p.y < rows - 1 && p.x < cols - 1)
                neighbours.push_back({.x = p.x + 1, .y = p.y + 1});
        if (p.x > 0 && p.y < rows - 1)
                neighbours.push_back({.x = p.x - 1, .y = p.y + 1});
        if (p.x < cols - 1 && p.y > 0)
                neighbours.push_back({.x = p.x + 1, .y = p.y - 1});

        return neighbours;
}

std::vector<IntPoint> get_adjacent_neighbours_inside_grid(const IntPoint &point,
                                                          int rows, int cols)
{
        std::vector<IntPoint> neighbours;
        // alias for readability;
        IntPoint p = point;

        // We add adjacent neighbours if within grid
        if (p.y > 0)
                neighbours.push_back({.x = p.x, .y = p.y - 1});
        if (p.y < rows - 1)
                neighbours.push_back({.x = p.x, .y = p.y + 1});
        if (p.x > 0)
                neighbours.push_back({.x = p.x - 1, .y = p.y});
        if (p.x < cols - 1)
                neighbours.push_back({.x = p.x + 1, .y = p.y});

        return neighbours;
}

std::vector<IntPoint> get_neighbours_toroidal_array(const IntPoint &point,
                                                    int rows, int cols)
{
        std::vector<IntPoint> neighbours;
        // alias for readability;
        IntPoint p = point;

        neighbours.push_back(
            {.x = p.x, .y = mathematical_modulo(p.y - 1, rows)});
        neighbours.push_back(
            {.x = p.x, .y = mathematical_modulo(p.y + 1, rows)});
        neighbours.push_back(
            {.x = mathematical_modulo(p.x - 1, cols), .y = p.y});
        neighbours.push_back(
            {.x = mathematical_modulo(p.x + 1, cols), .y = p.y});
        neighbours.push_back({.x = mathematical_modulo(p.x - 1, cols),
                              .y = mathematical_modulo(p.y - 1, rows)});
        neighbours.push_back({.x = mathematical_modulo(p.x + 1, cols),
                              .y = mathematical_modulo(p.y + 1, rows)});
        neighbours.push_back({.x = mathematical_modulo(p.x - 1, cols),
                              .y = mathematical_modulo(p.y + 1, rows)});
        neighbours.push_back({.x = mathematical_modulo(p.x + 1, cols),
                              .y = mathematical_modulo(p.y - 1, rows)});
        return neighbours;
}

bool is_adjacent(const IntPoint &p1, const IntPoint &p2)
{
        return (abs(p1.x - p2.x) <= 1 && abs(p1.y - p2.y) <= 1);
}

/* === Point Implementations === */

/**
 * Scalar product
 */
Point Point::operator*(double scalar) { return {scalar * x, scalar * y}; }
/**
 * Dot product
 */
double Point::operator*(Point other) { return x * other.x + y * other.y; }

IntPoint Point::cast() { return IntPoint{(int)x, (int)y}; }

Point operator+(const Point &p1, const Point &p2)
{
        return {p1.x + p2.x, p1.y + p2.y};
}
Point operator-(const Point &p1, const Point &p2)
{
        return {p1.x - p2.x, p1.y - p2.y};
}

double distance(const Point &p1, const Point &p2)
{
        double dx = p1.x - p2.x;
        double dy = p1.y - p2.y;
        return sqrt(dx * dx + dy * dy);
}

/* === LineSegment Implementations === */

/**
 * Checks if a point lies on the line segment. For now we assume segments are
 * either horizontal or vertical.
 */
bool LineSegment::contains(const Point &p) const
{
        // for now we assume segments are either only horizontal or
        // vertical.
        double eps = 0.01;
        bool on_the_line, within_bounds;
        if (is_horizontal()) {
                on_the_line = start.y - eps < p.y && p.y < start.y + eps;
                within_bounds = start.x <= p.x && p.x <= end.x;
        } else {
                on_the_line = start.x - eps < p.x && p.x < start.x + eps;
                within_bounds = start.y <= p.y && p.y <= end.y;
        }
        return on_the_line && within_bounds;
}

bool LineSegment::is_horizontal() const { return start.y == end.y; }

bool LineSegment::is_vertical() const { return start.x == end.x; }

/* === Rectangle Implementations === */

bool Rectangle::contains(const Point &p) const
{
        return (top_left.x <= p.x && p.x <= top_left.x + width) &&
               (top_left.y <= p.y && p.y <= top_left.y + height);
}

/**
 * Returns the four edges starting from the top left corner and going
 * clockwise.
 */
std::vector<LineSegment> Rectangle::get_edges() const
{

        Point width_vector = {width, 0};
        Point height_vector = {0, height};
        Point top_right = top_left + width_vector;
        Point bottom_left = top_left + height_vector;
        Point bottom_right = top_right + height_vector;

        return {
            {top_left, top_right},
            {top_right, bottom_right},
            {bottom_right, bottom_left},
            {bottom_left, top_left},
        };
}

LineSegment Rectangle::get_top_edge() const { return get_edges()[0]; }
LineSegment Rectangle::get_bottom_edge() const { return get_edges()[2]; }
LineSegment Rectangle::get_left_edge() const { return get_edges()[3]; }
LineSegment Rectangle::get_right_edge() const { return get_edges()[1]; }

/* === Circle Implementations === */

bool Circle::contains(const Point &p) const
{
        return distance(center, p) <= radius;
}

/* === Interaction Implementations === */

bool collides(const Circle &c, const LineSegment &l)
{
        // Check if either endpoint of the line segment is inside the circle
        if (c.contains(l.start) || c.contains(l.end)) {
                return true;
        }

        // Calculate the projection of the circle's center onto the line segment
        Point line_vec = l.end - l.start;
        Point center_vec = c.center - l.start;

        // projection scaling factor t is the dot product of center_vec and
        // line_vec divided by the dot product of line_vec with itself
        double t = (center_vec * line_vec) / (line_vec * line_vec);

        // Clamp t to the range [0, 1] to stay within the segment
        t = std::max(0.0, std::min(1.0, t));

        // Apply the scaling to find the actual projection.
        Point closest_point = l.start + line_vec * t;

        // Check if this closest point is within the circle
        return c.contains(closest_point);
}

bool collides(const Circle &c, const Rectangle &rect)
{
        if (rect.contains(c.center))
                return true;

        for (const auto &edge : rect.get_edges()) {
                if (collides(c, edge))
                        return true;
        }

        return false;
}
