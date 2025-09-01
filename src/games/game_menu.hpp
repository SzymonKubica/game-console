#include "../common/platform/interface/platform.hpp"
#include "game_executor.hpp"

typedef enum Game
    : int { Unknown = 0,
            MainMenu = 1,
            Clean2048 = 2,
            Minesweeper = 3,
            GameOfLife = 4,
            Settings = 5,
            RandomSeedPicker = 6,
    } Game;

typedef struct GameMenuConfiguration {
        Game game;
        Color accent_color;
} GameMenuConfiguration;

extern Game map_game_from_str(const char *name);

extern const char *map_game_to_str(Game game);

void select_game(Platform *p);

/**
 * Similar to `collect_configuration` from `configuration.hpp`, it returns true
 * if the configuration was successfully collected. Otherwise, if the user requested
 * exit by pressing the blue button, it returns false and this needs to be
 * handled by the main game loop.
 */
bool collect_game_configuration(Platform *p,
                                GameMenuConfiguration *configuration);
