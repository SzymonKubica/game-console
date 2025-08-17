#include "../common/platform/interface/platform.hpp"
#include "game_executor.hpp"
#include <cstdint>

typedef enum Game: int8_t {
        Unknown = -1,
        MainMenu = 0,
        Clean2048 = 1,
        Minesweeper = 2,
        GameOfLife = 3,
        Settings = 4,
} Game;

extern Game map_game_from_str(const char *name);

void select_game(Platform *p);

void collect_game_configuration(Platform *p, Game *selected_game,
                                GameCustomization *customization);

