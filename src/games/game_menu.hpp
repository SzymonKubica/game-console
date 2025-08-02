#include "../common/platform/interface/platform.hpp"

typedef enum Game {
        Unknown = -1,
        Clean2048 = 0,
        Snake = 1,
        Minesweeper = 2,
        GameOfLife = 3,
} Game;

extern Game map_game_from_str(const char *name);

void select_game(Platform *p);

