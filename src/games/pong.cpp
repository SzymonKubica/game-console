#include <stdbool.h>
#include <string.h>
#include <cstring>
#include <string>
#include <cassert>
#include "pong.hpp"

#include "../common/logging.hpp"
#include "../common/constants.hpp"
#include "../common/grid.hpp"
#include "../platform/interface/display.hpp"
#include "../platform/interface/platform.hpp"
#include "../common/configuration.hpp"

#include "../menu.hpp"
#include "../common/common_transitions.hpp"
#include "../apps/settings.hpp"

#define TAG "Pong"

PongConfiguration DEFAULT_PONG_GAME_CONFIG = {
    .header = ConfigurationHeader(),
};

const char *Pong::get_game_name() const { return "Pong"; }
const char *Pong::get_help_text() const { return "TODO"; }

void draw_pong_canvas(const Platform &p,
                      const SquareCellGridDimensions &dimensions,
                      const UserInterfaceCustomization &customization)

{
        p.display->initialize();
        p.display->clear(Black);

        if (customization.rendering_mode == Detailed)
                p.display->draw_rounded_border(customization.accent_color);

        int x_margin = dimensions.left_horizontal_margin;
        int y_margin = dimensions.top_vertical_margin;

        int actual_width = dimensions.actual_width;
        int actual_height = dimensions.actual_height;

        int border_width = 2;
        // We need to make the border rectangle and the canvas slightly
        // bigger to ensure that it does not overlap with the game area.
        // Otherwise the caret rendering erases parts of the border as
        // it moves around (as the caret intersects with the border
        // partially)
        int border_offset = 1;

        /* We don't draw the individual rectangles to make rendering
           faster on the physical Arduino LCD display. */
        p.display->clear_region(
            {.x = x_margin - border_offset, .y = y_margin - border_offset},
            {.x = x_margin + actual_width + border_offset,
             .y = y_margin + actual_height + border_offset},
            Black);

        p.display->draw_rectangle(
            {.x = x_margin - border_offset, .y = y_margin - border_offset},
            actual_width + 2 * border_offset, actual_height + 2 * border_offset,
            customization.accent_color, border_width, false);

        if (customization.show_help_text) {
                std::map<Action, std::string> button_hints;
                button_hints[BACK_ACTION] = "Quit";
                button_hints[CONFIRM_ACTION] = "TODO";
                button_hints[FORWARD_ACTION] = "Pause";
                button_hints[HELP_ACTION] = "Help";
                render_controls_explanations(*p.display,
                                             p.capabilities.action_button_kind,
                                             button_hints);
        }
}

struct Ball {
        Point position;
        Point velocity;
};

struct Segment {
        Point start;
        Point end;

        bool contains(const Point &p) const
        {
                // for now we assume segments are either only horizontal or
                // vertical.
                double eps = 0.01;
                bool on_the_line, within_bounds;
                if (is_horizontal()) {
                        on_the_line =
                            start.y - eps < p.y && p.y < start.y + eps;
                        within_bounds = start.x <= p.x && p.x <= end.x;
                } else {
                        on_the_line =
                            start.x - eps < p.x && p.x < start.x + eps;
                        within_bounds = start.y <= p.y && p.y <= end.y;
                }
                return on_the_line && within_bounds;
        }

        bool is_horizontal() const { return start.y == end.y; }

        bool is_vertical() const { return start.x == end.x; }
};

UserAction Pong::app_loop(const Platform &p,
                          const UserInterfaceCustomization &customization,
                          const PongConfiguration &config) const
{

        int game_cell_width = 2;
        auto gd =
            std::unique_ptr<SquareCellGridDimensions>(calculate_grid_dimensions(
                p.display->get_width(), p.display->get_height(),
                p.display->get_display_corner_radius(), game_cell_width));
        int rows = gd->rows;
        int cols = gd->cols;

        draw_pong_canvas(p, *gd, customization);

        // We need to locate the grid vertices to assemble wall segments.
        int radius = 3;
        // this padding is needed so that the ball doesn't clip the walls of
        // the game grid.
        int padding = radius;
        Point top_left = {(double)gd->left_horizontal_margin + padding,
                          (double)gd->top_vertical_margin + padding};
        Point top_right =
            top_left + Point{(double)gd->actual_width - 2 * padding, 0};
        Point bottom_left =
            top_left + Point{0, (double)gd->actual_height - 2 * padding};
        Point bottom_right =
            top_right + Point{0, (double)gd->actual_height - 2 * padding};

        Segment top_wall{top_left, top_right};
        Segment bottom_wall{bottom_left, bottom_right};
        Segment left_wall{top_left, bottom_left};
        Segment right_wall{top_right, bottom_right};

        // define the paddle segment

        int paddle_len = gd->actual_height / 4;
        int paddle_w = 5;
        // some intense maths here to make the paddle centered.
        Point paddle_start = {top_left.x + padding + paddle_w,
                              top_left.y +
                                  (gd->actual_height - 2 * padding) / 2.0 -
                                  paddle_len / 2.0};
        Point paddle_end = paddle_start + Point{0, (double)paddle_len};

        Segment paddle{paddle_start, paddle_end};
        // for collision we need a segment that lies on the actual boundary
        // after the rectangle is drawn.
        double surface_offset = (double)paddle_w + padding;
        Segment paddle_surface{paddle_start + Point{surface_offset, 0},
                               paddle_end + Point{surface_offset, 0}};

        std::vector<Segment *> walls = {&paddle_surface, &top_wall,
                                        &bottom_wall, &left_wall, &right_wall};

        // we do some simulation here
        Point pos = {gd->actual_width / 2.0, gd->actual_height / 2.0};
        Point v = {1.0, 1.0};
        Ball ball{pos, v};
        int time_delta = 10; // ms

        auto render_paddle = [&](Segment paddle) {
                p.display->draw_rectangle(paddle.start.cast(), paddle_w,
                                          paddle_len,
                                          customization.accent_color, 1, false);
        };

        auto erase_paddle = [&](Segment paddle) {
                p.display->draw_rectangle(paddle.start.cast(), paddle_w,
                                          paddle_len, Black, 1, false);
        };

        render_paddle(paddle);

        // Note on increasing ball velocity: right now the collision detection
        // is crap if the ball moves too fast, there is a chance that it will
        // never hit the seg.contains() check. We need some continuous collision
        // detection mechanism to allow for increasing the game speed beyond
        // 1pixel per tick

        bool game_over = false;
        while (!game_over) {
                auto maybe_action = poll_action_input(p.action_controllers);
                if (maybe_action.has_value() &&
                    maybe_action.value() == BACK_ACTION) {
                        break;
                }

                auto maybe_direction =
                    poll_directional_input(p.directional_controllers);
                if (maybe_direction.has_value()) {
                        auto dir = maybe_direction.value();
                        if (dir == Direction::UP || dir == Direction::DOWN) {

                                Point offset;
                                if (dir == Direction::UP) {
                                        offset = {0, -2 * v.y};
                                } else {
                                        offset = {0, 2 * v.y};
                                }

                                bool out_of_bounds = false;

                                // prevent paddle from going out of bounds.
                                out_of_bounds |=
                                    dir == Direction::UP &&
                                    paddle.start.y - v.y <= top_wall.start.y;
                                out_of_bounds |=
                                    dir == Direction::DOWN &&
                                    paddle.end.y + v.y >= bottom_wall.end.y;

                                if (!out_of_bounds) {
                                        erase_paddle(paddle);
                                        paddle.start = paddle.start + offset;
                                        paddle.end = paddle.end + offset;
                                        // This is important it affects the
                                        // actual segment used for colision
                                        // detection against the paddle. Ideally
                                        // this should be baked into the
                                        // rectangle object representing the
                                        // paddle so that we don't need to track
                                        // this separately.
                                        paddle_surface.start =
                                            paddle_surface.start + offset;
                                        paddle_surface.end =
                                            paddle_surface.end + offset;
                                        render_paddle(paddle);
                                }
                        }
                }

                // erase the previous location
                p.display->draw_circle(ball.position.cast(), radius, Black, 1,
                                       true);

                // take a step
                ball.position = ball.position + ball.velocity;

                for (const auto &seg : walls) {
                        if (!seg->contains(ball.position))
                                continue;
                        if (seg == &left_wall) {
                                game_over = true;
                        }

                        // collision detected
                        // roll back the previous step
                        ball.position = ball.position - ball.velocity;

                        if (seg->is_horizontal())
                                ball.velocity.y = -ball.velocity.y;
                        if (seg->is_vertical())
                                ball.velocity.x = -ball.velocity.x;
                }

                p.display->draw_circle(ball.position.cast(), radius, Red, 1,
                                       true);

                if (!p.display->refresh())
                        return UserAction::CloseWindow;
                p.time_provider->delay_ms(time_delta);
        }

        wait_until_green_pressed(p);
        return UserAction::PlayAgain;
}

PongConfiguration *load_initial_pong_config(const PersistentStorage &storage)
{
        int storage_offset = get_settings_storage_offset(Game::Pong);

        PongConfiguration config;
        LOG_DEBUG(TAG,
                  "Trying to load initial settings from the persistent storage "
                  "at offset %d",
                  storage_offset);
        storage.get(storage_offset, config);

        PongConfiguration *output = new PongConfiguration();

        if (!config.header.validate_against(DEFAULT_PONG_GAME_CONFIG)) {
                LOG_DEBUG(TAG,
                          "The storage does not contain a valid "
                          "pong game configuration, using default values.");
                memcpy(output, &DEFAULT_PONG_GAME_CONFIG,
                       sizeof(PongConfiguration));
                storage.put(storage_offset, DEFAULT_PONG_GAME_CONFIG);

        } else {
                LOG_DEBUG(TAG, "Using configuration from persistent storage.");
                memcpy(output, &config, sizeof(PongConfiguration));
        }

        return output;
}

/**
 * Assembles the generic configuration struct that is needed to collect
 * user defined game configuration for pong. Note that this is a
 * declarative way of defining what can be configured and the UI code
 * then dynamically renders selectors and handles switching between
 * option values.
 *
 * WARNING: This is tightly coupled with the
 * `extract_game_config` function. If you change the
 * structure of this config, make sure to make a corresponding update to
 * that function below to ensure that the specific game config can be
 * successfully extracted from the generic config struct.
 */
Configuration *assemble_pong_configuration(PersistentStorage *storage,
                                           PongConfiguration *initial_config)
{
        auto *initial_speed = ConfigurationOption::of_integers(
            "Speed", {3, 4, 5}, initial_config->initial_speed);

        std::vector<ConfigurationOption *> options = {initial_speed};

        return new Configuration("Pong", options);
}
void extract_game_config(PongConfiguration &game_config,
                         const PongConfiguration &initial_config,
                         const Configuration &config)
{
        ConfigurationOption initial_speed = *config.options[0];
        game_config.initial_speed = initial_speed.get_curr_int_value();
}

std::optional<UserAction>
Pong::collect_config(const Platform &p,
                     const UserInterfaceCustomization &customization,
                     PongConfiguration &game_config) const
{
        auto initial_cfg = std::unique_ptr<PongConfiguration>(
            load_initial_pong_config(*p.persistent_storage));
        auto cfg = std::unique_ptr<Configuration>(assemble_pong_configuration(
            p.persistent_storage, initial_cfg.get()));

        auto interrupt = collect_configuration(p, *cfg, customization);
        if (interrupt)
                return interrupt;
        extract_game_config(game_config, *initial_cfg, *cfg);
        return std::nullopt;
}
