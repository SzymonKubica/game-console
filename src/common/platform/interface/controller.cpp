#include "controller.hpp"

/**
 * Checks if any of the controllers has recorded user input. If so, the input
 * direction will be written into the `registered_dir` output parameter.
 */
bool directional_input_registered(
    std::vector<DirectionalController *> *controllers,
    Direction *registered_dir)
{
        bool input_registered = false;
        for (DirectionalController *controller : *controllers) {
                input_registered |= controller->poll_for_input(registered_dir);
        }
        return input_registered;
}

bool action_input_registered(std::vector<ActionController *> *controllers,
                             Action *registered_action)
{
        bool input_registered = false;
        for (ActionController *controller : *controllers) {
                input_registered |=
                    controller->poll_for_input(registered_action);
        }
        return input_registered;
}
