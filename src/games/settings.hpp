#pragma once
#include "game_executor.hpp"
#include "common_transitions.hpp"
#include "../common/logging.hpp"
#include "../common/configuration.hpp"

/**
 * Similar to `collect_configuration` from `configuration.hpp`, it returns true
 * if the configuration was successfully collected. Otherwise, if the user
 * requested exit by pressing the blue button, it returns false and this needs
 * to be handled by the main game loop.
 */
bool enter_settings_loop(Platform *platform, GameCustomization *customization);

std::vector<int> get_settings_storage_offsets();

/**
 * This 'game' is a settings menu responsible for setting the default values of
 * all config options for all games. The idea is that this game allows for
 * chosing the game for which we want to set the default settings, then renders
 * the game configuration menu and allows the user to set the defaults using the
 * same UI as they would use before starting the game. The chosen settings are
 * then saved in the persistent storage and used as the default values in the
 * future.
 */
class Settings : public GameExecutor
{
      public:
        void enter_game_loop(Platform *p,
                             GameCustomization *customization) override
        {
                while (enter_settings_loop(p, customization)) {
                        LOG_DEBUG("settings",
                                  "Re-entering the settings collecting loop.")
                }
        }

        Settings() {}
};
