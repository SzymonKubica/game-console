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
                button_hints[CONFIRM_ACTION] = "Continue";
                button_hints[FORWARD_ACTION] = "Pause";
                button_hints[HELP_ACTION] = "Help";
                render_controls_explanations(*p.display,
                                             p.capabilities.action_button_kind,
                                             button_hints);
        }
}

struct Ball {
        Circle circle;
        Point velocity;
};

struct Paddle {
        Rectangle body;
        Point velocity;
        Point acceleration;
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
        int padding = 1;
        int game_area_width = gd->actual_width - 2 * padding;
        int game_area_height = gd->actual_height - 2 * padding;
        Point top_left = {(double)gd->left_horizontal_margin + padding,
                          (double)gd->top_vertical_margin + padding};
        Point top_right =
            top_left + Point{(double)gd->actual_width - 2 * padding, 0};
        Point bottom_left =
            top_left + Point{0, (double)gd->actual_height - 2 * padding};
        Point bottom_right =
            top_right + Point{0, (double)gd->actual_height - 2 * padding};

        LineSegment top_wall{top_left, top_right};
        LineSegment bottom_wall{bottom_left, bottom_right};
        LineSegment left_wall{top_left, bottom_left};
        LineSegment right_wall{top_right, bottom_right};

        int paddle_len = gd->actual_height / 4;
        int paddle_w = 5;
        // some intense maths here to make the paddle centered.
        Point paddle_start = {top_left.x + padding + paddle_w,
                              top_left.y +
                                  (gd->actual_height - 2 * padding) / 2.0 -
                                  paddle_len / 2.0};
        Point paddle_end = paddle_start + Point{0, (double)paddle_len};

        Paddle paddle{
            .body = {paddle_start, (double)paddle_w, (double)paddle_len},
            .velocity = {0, 0},
            .acceleration = {0.1, 0.1},
        };

        Point cpu_paddle_offset{(double)gd->actual_width -
                                    2 * ((double)paddle_w + padding) - paddle_w,
                                0};

        Paddle cpu_paddle{
            .body = {paddle_start + cpu_paddle_offset, (double)paddle_w,
                     (double)paddle_len},
            .velocity = {0, 0},
            .acceleration = {0.1, 0.1},
        };

        std::vector<LineSegment *> walls = {&top_wall, &bottom_wall, &left_wall,
                                            &right_wall};

        Point pos = {gd->actual_width / 2.0, gd->actual_height / 2.0};
        double initial_velocity = 1.0;
        Point v = {initial_velocity, initial_velocity};
        double friction = 0.25;
        Ball ball{Circle{pos, (double)radius}, v};
        int time_delta = 1000 / config.initial_speed; // ms

        // Rendering lambdas to make the logic code more readable.
        auto erase_paddle = [&](Rectangle paddle) {
                p.display->draw_rectangle(paddle.top_left.cast(), paddle_w,
                                          paddle_len, Black, 1, false);
        };
        auto render_paddle = [&](Rectangle paddle) {
                p.display->draw_rectangle(paddle.top_left.cast(), paddle_w,
                                          paddle_len,
                                          customization.accent_color, 1, false);
        };
        auto erase_ball = [&](Ball ball) {
                p.display->draw_circle(ball.circle.center.cast(), radius, Black,
                                       1, true);
        };
        auto render_ball = [&](Ball ball) {
                p.display->draw_circle(ball.circle.center.cast(), radius, Red,
                                       1, true);
        };

        render_paddle(paddle.body);
        render_paddle(cpu_paddle.body);

        int expected_impact_y =
            (int)ball.circle.center.y +
            ((int)(ball.velocity.y * (game_area_width / ball.velocity.x))) %
                game_area_height;

        bool game_over = false;
        bool game_paused = false;
        bool action_input_on_last_iteration = false;
        while (!game_over) {
                auto maybe_action = poll_action_input(p.action_controllers);
                if (maybe_action.has_value() &&
                    maybe_action.value() == BACK_ACTION) {
                        break;
                }
                if (maybe_action.has_value() &&
                    maybe_action.value() == FORWARD_ACTION &&
                    !action_input_on_last_iteration) {
                        game_paused = !game_paused;
                        action_input_on_last_iteration = true;
                        p.time_provider->delay_ms(INPUT_POLLING_DELAY);
                }

                if (!maybe_action.has_value())
                        action_input_on_last_iteration = false;
                if (game_paused) {
                        p.time_provider->delay_ms(INPUT_POLLING_DELAY);
                        continue;
                }

                auto maybe_direction =
                    poll_directional_input(p.directional_controllers);
                if (maybe_direction.has_value()) {
                        auto dir = maybe_direction.value();
                        if (dir == Direction::UP || dir == Direction::DOWN) {
                                int dir_sign = dir == Direction::UP ? -1 : 1;
                                paddle.velocity.y +=
                                    dir_sign * paddle.acceleration.y;
                                Point off = {0, paddle.velocity.y};

                                // prevent paddle from going out of bounds.
                                bool outside = false;
                                double new_top, new_bottom;
                                new_top = paddle.body.top_left.y + off.y;
                                outside |= new_top <= top_wall.start.y;
                                new_bottom = paddle.body.top_left.y + off.y +
                                             paddle.body.height;
                                outside |= new_bottom >= bottom_wall.end.y;

                                if (!outside) {
                                        erase_paddle(paddle.body);
                                        paddle.body.top_left =
                                            paddle.body.top_left + off;
                                        render_paddle(paddle.body);
                                } else {
                                        paddle.velocity.y = 0;
                                }
                        }
                } else {
                        // For now we do no deceleration.
                        paddle.velocity = {0, 0};
                }

                // Handle cpu paddle.
                erase_paddle(cpu_paddle.body);
                if (cpu_paddle.body.top_left.y > expected_impact_y) {
                        cpu_paddle.body.top_left.y -= initial_velocity;
                }
                if (cpu_paddle.body.top_left.y + cpu_paddle.body.height <
                    expected_impact_y) {
                        cpu_paddle.body.top_left.y += initial_velocity;
                }
                render_paddle(cpu_paddle.body);

                erase_ball(ball);

                ball.circle.center = ball.circle.center + ball.velocity;

                // collision detection
                for (const auto &seg : walls) {
                        if (!collides(ball.circle, *seg))
                                continue;
                        if (seg == &left_wall || seg == &right_wall)
                                game_over = true;
                        if (seg->is_horizontal())
                                ball.velocity.y = -ball.velocity.y;
                        if (seg->is_vertical())
                                ball.velocity.x = -ball.velocity.x;
                }
                if (collides(ball.circle, paddle.body)) {
                        ball.velocity.x = -ball.velocity.x;
                        // the velocity of the paddle is partially tranferred to
                        // the vertical velocity of the ball. This is controlled
                        // by the friction coefficient.
                        ball.velocity.y += paddle.velocity.y * friction;
                        // we calculate the expected ball location here for the
                        // cpu paddle.
                        int expected_impact_y =
                            ball.circle.center.y + ((int)(ball.velocity.y *
                                   (game_area_width / ball.velocity.x))) %
                            game_area_height;
                }

                if (collides(ball.circle, cpu_paddle.body)) {
                        ball.velocity.x = -ball.velocity.x;
                        ball.velocity.y += cpu_paddle.velocity.y * friction;
                }

                render_ball(ball);

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
            "Speed (px/s)", {100, 150, 200, 250},
            initial_config->initial_speed);

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
