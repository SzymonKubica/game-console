#pragma once
#include "game_executor.hpp"
#include "common_transitions.hpp"
#include "../common/logging.hpp"
#include "../common/configuration.hpp"


void enter_settings_loop(Platform *platform, GameCustomization *customization);

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
                while (true) {
                        try {
                                enter_settings_loop(p, customization);
                        } catch (const ConfigurationLoopExitException &e) {
                                LOG_INFO("settings", "User requested exit in the game "
                                              "config loop.");
                                return;
                        }
                }
        }

        Settings() {}
};
