#include "../common/platform/interface/platform.hpp"
#include "game_executor.hpp"

typedef enum Game
    : int { Unknown = 0,
            MainMenu = 1,
            Clean2048 = 2,
            Minesweeper = 3,
            GameOfLife = 4,
            Settings = 5,
    } Game;

typedef struct GameMenuConfiguration {
        Game game;
        Color accent_color;
} GameMenuConfiguration;

extern Game map_game_from_str(const char *name);

void select_game(Platform *p);

void collect_game_configuration(Platform *p,
                                GameMenuConfiguration *configuration);
