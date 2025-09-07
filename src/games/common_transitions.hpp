#include "../common/platform/interface/display.hpp"
#include "../common/platform/interface/controller.hpp"
#include "../common/platform/interface/delay.hpp"
#include "../common/user_interface_customization.hpp"

void draw_game_over(Display *display, UserInterfaceCustomization *customization);
void draw_game_won(Display *display, UserInterfaceCustomization *customization);

void pause_until_any_directional_input(
    std::vector<DirectionalController *> *controllers,
    DelayProvider *delay_provider);
void pause_until_input(std::vector<DirectionalController *> *controllers,
                       std::vector<ActionController *> *action_controllers,
                       Direction *direction, Action *action,
                       DelayProvider *delay_provider);
