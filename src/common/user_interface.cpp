#include "user_interface.h"
#include "configuration.hpp"
#include "platform/interface/color.hpp"
#include "platform/interface/display.hpp"
#include "../common/logging.hpp"
#include "constants.hpp"
#include <cassert>
#include <cstdio>
#include <cstring>

#define GRID_BG_COLOR White
#define TAG "user_interface"

/* User Interface */

/* Helper functions used by draw_configuration_menu */

inline int get_centering_margin(int screen_width, int text_length,
                                int font_width);
void render_config_bar_centered(Display *display, int y_start,
                                int option_text_max_len, int value_text_max_len,
                                const char *option_text, const char *value_text,
                                bool is_already_rendered,
                                bool update_value_cell,
                                Color cell_bg_color = DarkBlue);
void render_text_bar_centered(Display *display, int y_start,
                              int option_text_max_len, int value_text_max_len,
                              const char *text, bool is_already_rendered,
                              Color bg_color = DarkBlue,
                              Color text_color = White,
                              int font_width = FONT_WIDTH,
                              FontSize font_size = Size16);
void render_circle_selector(Display *display, bool already_rendered, int x_axis,
                            int *y_positions, int y_positions_len,
                            int prev_pos_idx, int curr_pos_idx, int radius,
                            Color bg_color = Black,
                            Color circle_color = DarkBlue);
int calculate_section_spacing(int display_height, int config_bar_num,
                              int bar_height, int gap_between_bars_height,
                              FontSize heading_font_size);
int *calculate_config_bar_positions(int y_spacing, FontSize heading_font_size,
                                    int bar_height, int bar_gap_height,
                                    int config_bar_num);

/**
 *  Draws the config menu given the old and new config values
 *  The old config given in `previous_config` is needed to determine which
 *  parts of the UI need to be updated.
 *
 *  The overall layout of the config menu looks as follows:
 *                      ____________________
 *                     |       Heading      |
 *                      --------------------
 *              ___________________________________
 *             |    Config Option 1      | Value | |
 *              -----------------------------------
 *                      ...
 *              ___________________________________
 *             |    Config Option n      | Value | |
 *              -----------------------------------
 *
 *  The function calculates positions of the heading bar and all
 *  configuration bars. The nomenclature in the function variable names is
 *  as follows:
 *  - `config_bar` refers to the entire body of the configuration option bar
 *    refer to the diagram above.
 *  - `option_text` refers to the name of the configuration option on each
 *  bar
 *    e.g. 'Config Option 1' above.
 *  - `value_cell` refers to the cell on each of the config bars that holds
 *  the actual value that is being modified by the configuration bar.
 */
void render_config_menu(Display *display, GameConfiguration *config,
                        GameConfiguration *previous_config,
                        bool already_rendered)
{
        if (!already_rendered) {
                display->initialize();
                display->clear(Black);
        }
        /* Display strings */
        const char *heading_text = "2048";
        const char *grid_size_option_text = "Grid size:";
        const char *target_option_text = "Game target:";
        const char *start_text = "Start Game";

        int max_game_target_text_len = 4;
        int option_text_max_len = strlen(target_option_text);
        int option_value_separator = 2;
        int text_max_length = option_text_max_len + option_value_separator +
                              max_game_target_text_len;

        // We exctract the display dimensions and font sizes into shorter
        // variable names to make the code easier to read.
        int h = display->get_height();
        int w = display->get_width();
        int fw = FONT_WIDTH;
        int fh = FONT_SIZE;
        int left_margin = get_centering_margin(w, fw, text_max_length);

        int bar_height = 2 * fh;
        int config_bar_num = 3;
        int bar_gap_height = fh;
        int y_spacing = calculate_section_spacing(h, config_bar_num, bar_height,
                                                  bar_gap_height, Size24);

        int *bar_positions = calculate_config_bar_positions(
            y_spacing, Size24, bar_height, bar_gap_height, config_bar_num);
        int grid_size_bar_y = bar_positions[0];
        int target_config_bar_y = bar_positions[1];
        int start_text_bar_y = bar_positions[2];
        free(bar_positions);

        // We need to create string buffers for the values of the current config
        // options and pass them into the rendering functions so that they can
        // be displayed. This is done outside of the rendering function as we
        // want to have ability to render any config value (even strings) and
        // so it does not make sense to pass ints into the function and do
        // `sprintf` there.
        char grid_size_text[5];
        sprintf(grid_size_text, "%4d", config->grid_size);
        char target_text[5];
        sprintf(target_text, "%4d", config->target_max_tile);

        // Here we render all config/text bars for the 2048 game.
        render_text_bar_centered(display, y_spacing, option_text_max_len,
                                 max_game_target_text_len, heading_text,
                                 already_rendered, Black, White,
                                 HEADING_FONT_WIDTH, Size24);
        render_config_bar_centered(
            display, grid_size_bar_y, option_text_max_len,
            max_game_target_text_len, grid_size_option_text, grid_size_text,
            already_rendered, config->grid_size != previous_config->grid_size);
        render_config_bar_centered(
            display, target_config_bar_y, option_text_max_len,
            max_game_target_text_len, target_option_text, target_text,
            already_rendered,
            config->target_max_tile != previous_config->target_max_tile);
        render_text_bar_centered(display, start_text_bar_y, option_text_max_len,
                                 max_game_target_text_len, start_text,
                                 already_rendered);

        // Before we render the indicator dot we need to calculate its
        // positions. Note that the dot needs to appear exactly on the middle
        // axis of the config bars, because of this we need to add the
        // horizontal padding to the y positions of the config bars to center
        // the dot. This is to be considered for refactoring but currently this
        // pattern is not crystalized enough to abstract it.
        int padding = 1; // 0.5 fw on either side
        int bar_width = (text_max_length + padding) * fw;
        int circle_x = left_margin + bar_width + FONT_WIDTH;
        int h_padding = fh / 2;
        int circle_ys[] = {grid_size_bar_y + h_padding,
                           target_config_bar_y + h_padding,
                           start_text_bar_y + h_padding};
        int circle_ys_len = 3;
        int r = 5;

        render_circle_selector(display, already_rendered, circle_x, circle_ys,
                               circle_ys_len, previous_config->config_option,
                               config->config_option, r);
}

/**
 * Allows for rendering a centered configuration bar. A config bar consists of
 * two parts: the option text and the value cell. The option text displays the
 * name of the config option whereas the value cell shows the currently selected
 * value. It is supposed to render as follows:
 *   _____________________________________________
 *  /                     _______________________ \
 * |     Option text     |       Value cell      | |
 *  \                     ----------------------- /
 *   ---------------------------------------------
 * So the configuration bar is a large rounded rect that has the smaller rounded
 * rect for the actual value. It is very important that the background of the
 * value cell is white and the text is black. This is required by the specifics
 * of the display that we are using for the console. Given that it only allows
 * for redrawing one pixel at a time, and the speed of redrawing white/black is
 * the highest (less information required to transfer over the wire), we need
 * to opt for black-on-white text for parts of the game that get redrawn
 * frequently.
 *
 * The function takes the following parameters:
 * @param `display` A pointer to the implementation of the display to render on.
 * @param `y_start` The y coordinate of the top left corner of the config bar.
 * This function only enforces horizontal centering, the caller is responsible
 *        for enforcing proper vertical spacing using this argument.
 * @param `option_text_max_len` The maximum length of the option text, required
 * for centering. The idea here is that if you want to render multiple
 * configuration bars, you need to find the max across their option names and
 * pass it in when rendering each bar. This will ensure that all config bars are
 *        of the same width and are properly centered.
 * @param `value_text_max_len` The max length of the value text in the value
 * cell, similar to the `option_text_max_len` it is used for proper centering
 * and consistent spacing across multiple config bars rendered on the same UI.
 * @param `option_text` The text for the name of the config option to display on
 *        the config bar.
 * @param `is_already_rendered` Because of the limited speed of the display of
 *        the console, we need to be really efficient with what we render. If
 *        we want the config menu to be snappy, there is no point in redrawing
 *        the entire bar if the value cell changes. Because of this the caller
 *        needs to specify this flag to inform the function whether it is
 * supposed to render everything or just redraw the value cell.
 * @param `update_value_cell` If set, the function will only redraw the value
 * cell, the entire config bar will not be rerendered
 */
void render_config_bar_centered(Display *display, int y_start,
                                int option_text_max_len, int value_text_max_len,
                                const char *option_text, const char *value_text,
                                bool is_already_rendered,
                                bool update_value_cell, Color cell_bg_color)
{

        // For all selector buttons we need to find the one that has the longest
        // text and then put two spaces between the text of that one and the
        // selector option blob (the thing that displays the actual value of
        // the selected option).
        int option_value_gap_len = 2;
        // To center the text properly we need to get the maximum length in
        // characters of the text that will be displayed on the configuration
        // bars. The configuraion bars are of the form:
        // <option_name_text>__<value_text> with '__' denoting two spaces
        // between the option text and the value text.
        int text_len =
            option_text_max_len + option_value_gap_len + value_text_max_len;

        int h = display->get_height();
        int w = display->get_width();
        int fw = FONT_WIDTH;
        int fh = FONT_SIZE;

        int left_margin = get_centering_margin(w, fw, text_len);

        // We determine centering of the configuration bars based on the
        // position of the text. The actual rounded rect that contains the text
        // is slightly larger. Because of this we add 'padding' to the config
        // bar by making it start slightly to the left and up from the text. We
        // then make it longer and taller to properly contain all of the text.
        int h_padding = fw / 2;
        int v_padding = fh / 2;
        Point bar_start = {.x = left_margin - h_padding,
                           .y = y_start - v_padding};

        int bar_width = text_len * fw + 2 * h_padding;
        // The value cell is the small rounded rect inside of the config bar
        // that contains the actual value that the config bar is modifying.
        // Its rounded rect starts in the middle of the empty space between the
        // option text and the value text, hence we are dividing the separator
        // by two.
        int value_cell_x =
            left_margin + (option_text_max_len + option_value_gap_len / 2) * fw;
        // The value cell has to be smaller than the containing config bar,
        // because of this we add negative padding to make it fit.
        int value_cell_v_padding = fh / 4;
        int value_cell_y = y_start - value_cell_v_padding;
        Point value_cell_start = {.x = value_cell_x, .y = value_cell_y};

        if (!is_already_rendered) {
                // Draw the background for the two configuration cells.
                display->draw_rounded_rectangle(bar_start, bar_width, fh * 2,
                                                fh, cell_bg_color);
                // Draw the actual name of the config bar.
                Point grid_size_str_start = {.x = left_margin, .y = y_start};
                display->draw_string(grid_size_str_start, (char *)option_text,
                                     Size16, cell_bg_color, White);
        }

        // Draw / update the value of the cell
        if (!is_already_rendered || update_value_cell) {
                int value_cell_width = value_text_max_len * fw + 2 * h_padding;
                int value_cell_height = fh + v_padding;
                // We here we redraw the entire value cell (with the white
                // background so it is quite fast) If we need to render more
                // text we could make it more optimised to only redraw over the
                // place where actual characters are printed.
                display->draw_rounded_rectangle(
                    value_cell_start, value_cell_width, value_cell_height,
                    value_cell_height / 2, White);
                display->draw_string(
                    {.x = value_cell_start.x + h_padding,
                     .y = value_cell_start.y + value_cell_v_padding},
                    (char *)value_text, Size16, White, Black);
        }
}
/**
 * Similar to `render_config_bar_centered` refer to the documentation above
 * to understand the purpose of the function. The only difference is that this
 * is used for bars that have no value cells (e.g. bars displaying the name of
 * the game or the start 'button' bars).
 *
 * Given that there is on value cell that would require a re-render. This
 * function takes in only the `is_already_rendered` parameter. This is to be
 * correctly handled by the caller. That is, the function should be called with
 * this param as true only once per configuration collection session. The idea
 * is that since this text bar doesn't change, it makes no sense to re-render it
 * every time.
 */
void render_text_bar_centered(Display *display, int y_start,
                              int option_text_max_len, int value_text_max_len,
                              const char *text, bool is_already_rendered,
                              Color bg_color, Color text_color, int font_width,
                              FontSize font_size)
{

        // We calculate the total width using the same logic as for the config
        // bars. The reason we accept the same parameters is that the text bars
        // are to be created in the same context as the config bars so we'll
        // already have those max len params calculated there.
        int option_value_gap_len = 2;
        int text_len =
            option_text_max_len + option_value_gap_len + value_text_max_len;

        int h = display->get_height();
        int w = display->get_width();
        int fw = font_width;
        int fh = font_size;

        int left_margin = get_centering_margin(w, fw, text_len);

        int text_x = get_centering_margin(w, fw, strlen(text));

        // We determine centering of the configuration bars based on the
        // position of the text. The actual rounded rect that contains the text
        // is slightly larger. Because of this we add 'padding' to the config
        // bar by making it start slightly to the left and up from the text. We
        // then make it longer and taller to properly contain all of the text.
        int h_padding = fw / 2;
        int v_padding = fh / 2;
        Point bar_start = {.x = left_margin - h_padding,
                           .y = y_start - v_padding};

        int bar_width = text_len * fw + 2 * h_padding;

        if (!is_already_rendered) {
                // Draw the background for the two configuration cells.
                display->draw_rounded_rectangle(bar_start, bar_width, fh * 2,
                                                fh, bg_color);
                // Draw the actual text of the text bar. Note that it will be
                // centered inside of the large bar.
                Point grid_size_str_start = {.x = text_x, .y = y_start};
                display->draw_string(grid_size_str_start, (char *)text,
                                     font_size, bg_color, text_color);
        }
}

/**
 * Renders a small circle indicator in one of n given vertical positions. This
 * is used to indicate which config option is currently being edited. The
 * caller is responsible for calculating the positions of the circle indicators
 * and supplying the index of the previously selected position and the new one.
 *
 * The caller is responsible for correctly setting the `y_positions_len` to
 * match the supplied pointer to the `y_positions`, otherwise the function will
 * error with invalid array access.
 */
void render_circle_selector(Display *display, bool already_rendered, int x_axis,
                            int *y_positions, int y_positions_len,
                            int prev_pos_idx, int curr_pos_idx, int radius,
                            Color bg_color, Color circle_color)
{
        // We ignore the array overflow.
        if (prev_pos_idx >= y_positions_len ||
            curr_pos_idx >= y_positions_len) {
                return;
        }
        if (!already_rendered || prev_pos_idx != curr_pos_idx) {
                // First clear the old circle
                Point clear_pos = {.x = x_axis, .y = y_positions[prev_pos_idx]};
                display->draw_circle(clear_pos, radius, bg_color, 0, true);

                // Draw the new circle
                Point new_pos = {.x = x_axis, .y = y_positions[curr_pos_idx]};
                display->draw_circle(new_pos, radius, circle_color, 0, true);
        }
}

/**
 * Calculates the amount of spacing required so that the 3 following spacings
 * are equal:
 * - space from the top of the screen to the game / config title (heading)
 * - space from the heading to the config bars
 * - space from the config bars to the bottom of the screen.
 * We do this by taking the total height, subtacting the combined
 * height of all config bars + the game title (heading) and dividing
 * by 3 (the number of spacings)
 * inputs
 */
int calculate_section_spacing(int display_height, int config_bar_num,
                              int bar_height, int gap_between_bars_height,
                              FontSize heading_font_size)
{
        int spacings_num = 3;
        int total_gaps = config_bar_num - 1;
        int config_bars_height = config_bar_num * bar_height;
        int total_gaps_height = total_gaps * gap_between_bars_height;
        int total_config = config_bars_height + total_gaps_height;
        // Having calculated all intermediate heights, we get the final spacing.
        return (display_height - total_config - (int)heading_font_size) /
               spacings_num;
}
/**
 * Given the initial spacing in front of the config heading and the number,
 * sizes and gap size between the config bars, calculates the array of their y
 * positions and returns a pointer to it.
 *
 * The caller is responsible for freeing up this memory after the positions are
 * used.
 *
 */
int *calculate_config_bar_positions(int y_spacing, FontSize heading_font_size,
                                    int bar_height, int bar_gap_height,
                                    int config_bar_num)
{
        int heading_end = y_spacing + heading_font_size;

        int *bar_positions = (int *)malloc(config_bar_num * sizeof(int));

        int curr_bar_y = heading_end + y_spacing;
        for (int i = 0; i < config_bar_num; i++) {
                bar_positions[i] =
                    curr_bar_y + (bar_height + bar_gap_height) * i;
        }
        return bar_positions;
}

inline int get_centering_margin(int screen_width, int font_width,
                                int text_length)
{
        return (screen_width - text_length * font_width) / 2;
}

ConfigurationDiff *get_initial_no_diff()
{
        ConfigurationDiff *diff =
            static_cast<ConfigurationDiff *>(malloc(sizeof(ConfigurationDiff)));

        diff->option_switched = false;
        return diff;
}

Configuration *assemble_2048_game_menu_configuration()
{
        Configuration *config =
            static_cast<Configuration *>(malloc(sizeof(Configuration)));

        config->name = "2048";

        // Initialize the types of the config options.
        ConfigurationOptionType *configuration_option_types =
            static_cast<ConfigurationOptionType *>(
                malloc(2 * sizeof(ConfigurationOptionType)));
        configuration_option_types[0] = ConfigurationOptionType::INT;
        configuration_option_types[1] = ConfigurationOptionType::INT;
        config->type_map = configuration_option_types;

        // Initialize the first config option: game gridsize
        ConfigurationValue<int> *grid_size_config =
            static_cast<ConfigurationValue<int> *>(
                malloc(2 * sizeof(ConfigurationValue<int>)));
        grid_size_config->name = "Grid size:";
        grid_size_config->available_values =
            static_cast<int *>(malloc(3 * sizeof(int)));
        grid_size_config->available_values[0] = 3;
        grid_size_config->available_values[1] = 4;
        grid_size_config->available_values[2] = 5;
        grid_size_config->available_values_len = 3;
        grid_size_config->currently_selected = 1;
        grid_size_config->max_config_option_len = 1;

        ConfigurationValue<int> *game_target_config =
            static_cast<ConfigurationValue<int> *>(
                malloc(2 * sizeof(ConfigurationValue<int>)));
        game_target_config->name = "Game target:";
        game_target_config->available_values =
            static_cast<int *>(malloc(6 * sizeof(int)));
        game_target_config->available_values[0] = 128;
        game_target_config->available_values[1] = 256;
        game_target_config->available_values[2] = 512;
        game_target_config->available_values[3] = 1024;
        game_target_config->available_values[4] = 2048;
        game_target_config->available_values[5] = 4096;
        game_target_config->available_values_len = 6;
        game_target_config->currently_selected = 3;
        game_target_config->max_config_option_len = 4;

        config->config_values_len = 2;
        config->current_config_value = 1;
        config->configuration_values =
            static_cast<void **>(malloc(2 * sizeof(void *)));
        config->configuration_values[0] = grid_size_config;
        config->configuration_values[1] = game_target_config;
        config->confirmation_cell_text = "Start Game";
        return config;
}

/**
 *
 * @param `update` controlls if the config menu has already been rendered once
 * and only the text sections require updating. This is required on the physical
 * lcd display because redrawing the entire menu every time is too slow
 * so we need to be efficient about it.
 */
void render_generic_config_menu(Display *display, Configuration *config,
                                ConfigurationDiff *diff, bool update)
{

        bool already_rendered = update;
        int max_option_name_length =
            find_max_config_option_name_text_length(config);
        int max_option_value_length =
            find_max_config_option_value_text_length(config);
        int text_max_length =
            max_option_name_length + max_option_value_length + 2;

        LOG_DEBUG(TAG, "Found max text length across all config bars: %d",
                  text_max_length);
        int spacing =
            (display->get_height() - config->config_values_len * FONT_SIZE -
             HEADING_FONT_SIZE) /
            3;

        if (!update) {
                display->initialize();
                display->clear(Black);
        }

        const char *heading_text = config->name;

        // We exctract the display dimensions and font sizes into shorter
        // variable names to make the code easier to read.
        int h = display->get_height();
        int w = display->get_width();
        int fw = FONT_WIDTH;
        int fh = FONT_SIZE;
        int left_margin = get_centering_margin(w, fw, text_max_length);

        int bar_height = 2 * fh;
        int bar_gap_height = fh;
        int y_spacing = calculate_section_spacing(
            h, config->config_values_len, bar_height, bar_gap_height, Size24);

        /* We need to add one to the number of config bars below because of the
        confirmation button that is rendered at the bottom. */
        int *bar_positions = calculate_config_bar_positions(
            y_spacing, Size24, bar_height, bar_gap_height,
            config->config_values_len + 1);

        // Render the config menu heading.
        render_text_bar_centered(display, y_spacing, text_max_length, 0,
                                 heading_text, update, Black, White,
                                 HEADING_FONT_WIDTH, Size24);

        LOG_DEBUG(TAG, "Rendering %d config bars", config->config_values_len);

        for (int i = 0; i < config->config_values_len; i++) {
                int bar_y = bar_positions[i];
                if (config->type_map[i] == ConfigurationOptionType::INT) {
                        ConfigurationValue<int> *value =
                            static_cast<ConfigurationValue<int> *>(
                                config->configuration_values[i]);
                        // TODO: ensure that this number formatting logic works
                        // correctly.
                        char *option_text = value->name;
                        int selected_value =
                            value->available_values[value->currently_selected];
                        char option_value[value->max_config_option_len];
                        char format_string[4];
                        sprintf(format_string, "%%%dd",
                                max_option_value_length);
                        sprintf(option_value, format_string, selected_value);
                        render_config_bar_centered(
                            display, bar_y, max_option_name_length,
                            max_option_value_length, option_text, option_value,
                            update, diff->modified_option_index == i);
                } else {
                        ConfigurationValue<char *> *value =
                            static_cast<ConfigurationValue<char *> *>(
                                config->configuration_values[i]);

                        char *option_text = value->name;
                        char *option_value =
                            value->available_values[value->currently_selected];
                        render_config_bar_centered(
                            display, bar_y, max_option_name_length,
                            max_option_value_length, option_text, option_value,
                            update, diff->modified_option_index == i);
                }
        }

        int confirmation_cell_y = bar_positions[config->config_values_len];
        render_text_bar_centered(
            display, confirmation_cell_y, max_option_name_length,
            max_option_value_length, config->confirmation_cell_text,
            already_rendered);

        // Before we render the indicator dot we need to calculate its
        // positions. Note that the dot needs to appear exactly on the middle
        // axis of the config bars, because of this we need to add the
        // horizontal padding to the y positions of the config bars to center
        // the dot. This is to be considered for refactoring but currently this
        // pattern is not crystalized enough to abstract it.
        int padding = 1; // 0.5 fw on either side
        int bar_width = (text_max_length + padding) * fw;
        int circle_x = left_margin + bar_width + FONT_WIDTH;
        int h_padding = fh / 2;
        int circle_ys_len = config->config_values_len + 1;
        int r = 5;
        int circle_ys[circle_ys_len];
        for (int i = 0; i < circle_ys_len; i++) {
                circle_ys[i] = bar_positions[i] + h_padding;
        }

        render_circle_selector(display, already_rendered, circle_x, circle_ys,
                               circle_ys_len, diff->previously_edited_option,
                               diff->currently_edited_option, r);
        free(bar_positions);
}
