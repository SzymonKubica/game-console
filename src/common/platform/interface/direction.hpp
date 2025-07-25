#pragma once
/**
 * Enum modeling the four possible directions of user input.
 */
typedef enum Direction { UP = 0, RIGHT = 1, DOWN = 2, LEFT = 3 } Direction;

const char *direction_to_str(Direction direction);
