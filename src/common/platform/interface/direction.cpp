#include "direction.hpp"

const char *direction_to_string(Direction direction)
{
        switch (direction) {
        case UP:
                return "Up";
        case LEFT:
                return "Left";
        case RIGHT:
                return "Right";
        case DOWN:
                return "Down";
        default:
                return "Unknown";
        };
};
