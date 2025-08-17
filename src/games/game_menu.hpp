#include "../common/platform/interface/platform.hpp"

typedef enum Game {
        Unknown = -1,
        MainMenu = 0,
        Clean2048 = 1,
        Snake = 2,
        Minesweeper = 3,
        GameOfLife = 4,
        Settings = 5,
} Game;

extern Game map_game_from_str(const char *name);

void select_game(Platform *p);

