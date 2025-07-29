
#include "../common/platform/interface/display.hpp"
#include "../common/platform/interface/controller.hpp"
#include "../common/platform/interface/delay.hpp"

void draw_game_over(Display *display);
void draw_game_won(Display *display);
void pause_until_input(std::vector<DirectionalController *> *controllers,
                              DelayProvider *delay_provider);
