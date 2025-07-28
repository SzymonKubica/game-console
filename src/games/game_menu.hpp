#include "../common/platform/interface/color.hpp"
#include "../common/platform/interface/platform.hpp"

typedef enum Game {
        Unknown = -1,
        Clean2048 = 0,
        Snake = 1,
} Game;

extern Game map_game_from_str(const char *name);

void select_game(Platform *p);

typedef struct GameCustomization {
        Color border_color;
} GameCustomization;
