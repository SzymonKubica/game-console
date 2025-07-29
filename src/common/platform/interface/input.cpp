#include "input.hpp"

const char *direction_to_str(Direction direction)
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

const char *action_to_str(Action action)
{
        switch (action) {
        case YELLOW:
                return "Yellow";
        case RED:
                return "Red";
        case GREEN:
                return "Green";
        case BLUE:
                return "Blue";
        default:
                return "Unknown";
        };
};
