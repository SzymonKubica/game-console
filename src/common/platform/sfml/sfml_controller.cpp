#include "sfml_controller.hpp"

// TODO: this is a no-op for now, need to figure out how to get input from sfml
bool SfmlInputController::poll_for_input(Direction *input) {
  return false;
};

void SfmlInputController::setup() {};
