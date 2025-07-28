#include "interface/controller.hpp"

/**
 * Checks if any of the controllers has recorded user input. If so, the input
 * direction will be written into the `registered_dir` output parameter.
 */
bool input_registered(std::vector<Controller *> *controllers,
                      Direction *registered_dir)
{
        bool input_registered = false;
        for (Controller *controller : *controllers) {
                input_registered |= controller->poll_for_input(registered_dir);
        }
        return input_registered;
}
