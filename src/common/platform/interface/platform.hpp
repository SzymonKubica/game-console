#pragma once
#include "controller.hpp"
#include "delay.hpp"
#include "display.hpp"
#include <vector>

/**
 * Structure encapsulating all interfaces that a given implementation of the
 * game console platform needs to provide so that we can run our games on it.
 */
typedef struct Platform {
        Display *display;
        std::vector<DirectionalController*> *directional_controllers;
        std::vector<ActionController*> *action_controllers;
        DelayProvider *delay_provider;
} Platform;
