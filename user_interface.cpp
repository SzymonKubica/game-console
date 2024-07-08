#include "user_interface.h"
#include "GUI_Paint.h"
#include "LCD_Driver.h"
#include <cassert>
#include <cstdio>

/* Constants for configuring the UI. */
#define FONT_SIZE 16
#define FONT_WIDTH 11
#define TOP_LEFT_CORNER_X 23
#define TOP_LEFT_CORNER_Y 38

#define DISPLAY_CORNER_RADIUS 40
#define SCREEN_BORDER_WIDTH 3
#define GRID_BG_COLOR GRAY
#define DEFAULT_CELL_SPACING 10

int old_grid[4][4];

void initializeDisplay()
{
        Config_Init();
        LCD_Init();
        LCD_SetBacklight(100);

        for (int i = 0; i < 4; i++)
                for (int j = 0; j < 4; j++)
                        old_grid[i][j] = 0;

        Paint_Clear(BLACK);
}
/*******************************************************************************
  User Interface
*******************************************************************************/

typedef struct Point {
        int x;
        int y;
} Point;

/// Paints the white canvas for the game grid and the four blue
/// dots in the corners
static void drawRoundedBorder(int color);
static void drawRoundedRect(Point top_left, int width, int height, int radius,
                            int color);
static void drawGameGridSlots(int grid_size, int cell_spacing);
void drawGameCanvas(GameState *state)
{
        Paint_NewImage(LCD_WIDTH, LCD_HEIGHT, 270, WHITE);
        Paint_Clear(BLACK);
        drawRoundedBorder(DARKBLUE);
        Point top_left = {.x = 70, .y = 70};
        // drawRoundedRect(top_left, 100, 60, 20, WHITE);
        drawGameGridSlots(state->grid_size, DEFAULT_CELL_SPACING);
        return;
        int canvas_height = 10 * FONT_SIZE;
        int canvas_width = 21 * FONT_WIDTH;

        Point top_left_corner = {.x = TOP_LEFT_CORNER_X,
                                 .y = TOP_LEFT_CORNER_Y};

        Point bottom_right_corner = {.x = top_left_corner.x + canvas_width,
                                     .y = top_left_corner.y + canvas_height};

        int x_positions[2] = {top_left_corner.x, bottom_right_corner.x};
        int y_positions[2] = {top_left_corner.y, bottom_right_corner.y};

        Paint_ClearWindows(top_left_corner.x, top_left_corner.y,
                           bottom_right_corner.x, bottom_right_corner.y, WHITE);

        for (int x : x_positions) {
                for (int y : y_positions) {
                        Paint_DrawCircle(x, y, 6, BLUE, DOT_PIXEL_1X1,
                                         DRAW_FILL_FULL);
                }
        }
}

static void drawRoundedRect(Point start, int width, int height, int radius,
                            int color)
{

        Point top_left_corner = {.x = start.x + radius, .y = start.y + radius};

        Point bottom_right_corner = {.x = start.x + width - radius,
                                     .y = start.y + height - radius};

        int x_positions[2] = {top_left_corner.x, bottom_right_corner.x};
        int y_positions[2] = {top_left_corner.y, bottom_right_corner.y};

        // Draw the four rounded corners.
        for (int x : x_positions) {
                for (int y : y_positions) {
                        Paint_DrawCircle(x, y, radius, color, DOT_PIXEL_1X1,
                                         DRAW_FILL_FULL);
                }
        }

        Paint_DrawRectangle(top_left_corner.x, start.y,
                            start.x + width - radius, start.y + radius, color,
                            DOT_PIXEL_1X1, DRAW_FILL_FULL);

        Paint_DrawRectangle(start.x, top_left_corner.y, start.x + width + 1,
                            bottom_right_corner.y, color, DOT_PIXEL_1X1,
                            DRAW_FILL_FULL);

        // +1 is because the endY bound is not included
        Paint_DrawRectangle(top_left_corner.x, start.y + height - radius,
                            start.x + width - radius, start.y + height + 1,
                            color, DOT_PIXEL_1X1, DRAW_FILL_FULL);
}

/// Struct modelling all dimensional information required to properly render
/// and space out the grid slots that are used to display the game tiles.
typedef struct GridDimensions {
        int cell_height;
        int cell_width;
        int padding; // Padding added to the left of the grid if the available
                     // space isn't evenly divided into grid_size
        int grid_start_x;
        int grid_start_y;
        int score_cell_height;
        int score_cell_width;
        int score_start_x;
        int score_start_y;
} GridDimensions;

GridDimensions *calculateGridDimensions(int grid_size, int cell_spacing)
{
        GridDimensions *gd = (GridDimensions *)malloc(sizeof(GridDimensions));
        // HEIGHT and WIDTH are swapped because the display is mounted
        // horizontally. We subtract 4 times the border width to add padding
        // around the grid.
        int usable_width =
            LCD_HEIGHT - 2 * SCREEN_BORDER_WIDTH - 2 * cell_spacing;
        int usable_height = LCD_WIDTH - 2 * DISPLAY_CORNER_RADIUS;

        int cell_height =
            (usable_height - (grid_size - 1) * cell_spacing) / grid_size;
        int cell_width =
            (usable_width - (grid_size - 1) * cell_spacing) / grid_size;

        assert(cell_height >= FONT_SIZE);

        // We need to calculate the remainder width and then add a half of it
        // to the starting point to make the grid centered in case the usable
        // height doesn't divide evenly into grid_size.
        int remainder_width =
            (usable_width - (grid_size - 1) * cell_spacing) % grid_size;

        // We offset the grid downwards to allow it to overlap with the gap
        // between the two bottom corners and save space for the score at the
        // top of the grid.
        int corner_offset = DISPLAY_CORNER_RADIUS / 4;

        int grid_start_x =
            SCREEN_BORDER_WIDTH + cell_spacing + remainder_width / 2;
        int grid_start_y =
            SCREEN_BORDER_WIDTH + DISPLAY_CORNER_RADIUS + corner_offset;

        // We first draw a slot for the score
        int score_cell_width =
            LCD_HEIGHT -
            2 * (SCREEN_BORDER_WIDTH + cell_spacing + DISPLAY_CORNER_RADIUS);
        int score_cell_height = FONT_SIZE + cell_spacing;

        int score_start_y =
            (grid_start_y - score_cell_height - SCREEN_BORDER_WIDTH) / 2 +
            SCREEN_BORDER_WIDTH;

        Point score_start = {.x = SCREEN_BORDER_WIDTH + cell_spacing +
                                  DISPLAY_CORNER_RADIUS,
                             .y = score_start_y};

        int score_title_x = score_start.x + cell_spacing;
        int score_title_y = score_start.y + cell_spacing / 2;

        gd->cell_height = cell_height;
        gd->cell_width = cell_width;
        gd->padding = remainder_width;
        gd->grid_start_x = grid_start_x;
        gd->grid_start_y = grid_start_y;

        gd->score_cell_height = score_cell_height;
        gd->score_cell_width = score_cell_width;
        gd->score_start_x = score_start.x;
        gd->score_start_y = score_start.y;
}

/// Draws the background slots for the grid tiles, this takes a long time and
/// should only be called once at the start of the game to draw the grid. After
/// that drawGameGrid should be called to update the contents of the grid slots
/// with the numbers.
static void drawGameGridSlots(int grid_size, int cell_spacing)
{

        GridDimensions *gd = calculateGridDimensions(grid_size, cell_spacing);

        Point score_start = {.x = gd->score_start_x, .y = gd->score_start_y};
        drawRoundedRect(score_start, gd->score_cell_width,
                        gd->score_cell_height, gd->score_cell_height / 2,
                        GRID_BG_COLOR);

        char *buffer = "Score:";
        int score_title_x = score_start.x + cell_spacing;
        int score_title_y = score_start.y + cell_spacing / 2;

        Paint_DrawString_EN(score_title_x, score_title_y, buffer, &Font16,
                            GRID_BG_COLOR, BLACK);

        for (int i = 0; i < grid_size; i++) {
                for (int j = 0; j < grid_size; j++) {
                        Point start = {
                            .x = gd->grid_start_x +
                                 j * (gd->cell_width + cell_spacing),
                            .y = gd->grid_start_y +
                                 i * (gd->cell_height + cell_spacing)};

                        drawRoundedRect(start, gd->cell_width, gd->cell_height,
                                        gd->cell_height / 2, GRID_BG_COLOR);
                }
        }

        free(gd);
}

static void strReplace(char *str, char *oldWord, char *newWord);
static int number_string_length(int number);

void drawGameGrid(GameState *gs)
{
        int grid_size = gs->grid_size;
        int cell_spacing = DEFAULT_CELL_SPACING;

        GridDimensions *gd = calculateGridDimensions(grid_size, cell_spacing);

        int score_title_length = 6 * FONT_WIDTH;

        char score_buffer[20];
        sprintf(score_buffer, "%d", gs->score);

        int score_rounding_radius = gd->score_cell_height / 2;

        Paint_ClearWindows(
            gd->score_start_x + score_title_length + cell_spacing,
            gd->score_start_y + cell_spacing / 2,
            gd->score_start_x + gd->score_cell_width - score_rounding_radius,
            gd->score_start_y + cell_spacing / 2 + FONT_SIZE, GRID_BG_COLOR);

        Paint_DrawString_EN(gd->score_start_x + score_title_length + FONT_WIDTH,
                            gd->score_start_y + cell_spacing / 2, score_buffer,
                            &Font16, GRID_BG_COLOR, BLACK);

        for (int i = 0; i < grid_size; i++) {
                for (int j = 0; j < grid_size; j++) {
                        Point start = {
                            .x = gd->grid_start_x +
                                 j * (gd->cell_width + cell_spacing),
                            .y = gd->grid_start_y +
                                 i * (gd->cell_height + cell_spacing)};

                        if (gs->grid[i][j] != old_grid[i][j]) {
                                char *buffer = (char *)malloc(5 * sizeof(char));
                                sprintf(buffer, "%4d", gs->grid[i][j]);
                                strReplace(buffer, "   0", "    ");
                                // We need to center the four characters of text
                                // inside of the cell.
                                int x_margin =
                                    (gd->cell_width - 4 * FONT_WIDTH) / 2;
                                int y_margin =
                                    (gd->cell_height - FONT_SIZE) / 2;
                                int digit_len =
                                    number_string_length(old_grid[i][j]);

                                Paint_ClearWindows(
                                    start.x + x_margin +
                                        (4 - digit_len) * FONT_WIDTH,
                                    start.y + y_margin,
                                    start.x + x_margin + 4 * FONT_WIDTH,
                                    start.y + y_margin + FONT_SIZE,
                                    GRID_BG_COLOR);

                                Paint_DrawString_EN(
                                    start.x + x_margin, start.y + y_margin,
                                    buffer, &Font16, GRID_BG_COLOR, BLACK);
                                old_grid[i][j] = gs->grid[i][j];
                                free(buffer);
                        }
                }
        }
        free(gd);
}

static void drawRoundedBorder(int color)
{
        int rounding_radius = DISPLAY_CORNER_RADIUS;
        int margin = SCREEN_BORDER_WIDTH;
        int line_width = 2;
        Point top_left_corner = {.x = rounding_radius + margin,
                                 .y = rounding_radius + margin};
        Point bottom_right_corner = {.x = LCD_HEIGHT - rounding_radius - margin,
                                     .y = LCD_WIDTH - rounding_radius - margin};

        int x_positions[2] = {top_left_corner.x, bottom_right_corner.x};
        int y_positions[2] = {top_left_corner.y, bottom_right_corner.y};

        Paint_Clear(BLACK);

        // Draw the four rounded corners.
        for (int x : x_positions) {
                for (int y : y_positions) {
                        Paint_DrawCircle(x, y, rounding_radius, color,
                                         DOT_PIXEL_3X3, DRAW_FILL_EMPTY);
                }
        }

        // Draw the four lines connecting the circles.
        Paint_DrawLine(margin, top_left_corner.y, margin, bottom_right_corner.y,
                       color, DOT_PIXEL_3X3, LINE_STYLE_SOLID);

        Paint_DrawLine(LCD_HEIGHT - margin, top_left_corner.y,
                       LCD_HEIGHT - margin, bottom_right_corner.y, color,
                       DOT_PIXEL_3X3, LINE_STYLE_SOLID);

        Paint_DrawLine(top_left_corner.x, margin, bottom_right_corner.x, margin,
                       color, DOT_PIXEL_3X3, LINE_STYLE_SOLID);

        Paint_DrawLine(top_left_corner.x, LCD_WIDTH - margin,
                       bottom_right_corner.x, LCD_WIDTH - margin, color,
                       DOT_PIXEL_3X3, LINE_STYLE_SOLID);

        // Erase the middle bits of the four circles
        Paint_ClearWindows(margin + line_width, top_left_corner.y,
                           margin + line_width + 2 * rounding_radius,
                           top_left_corner.y + rounding_radius + line_width,
                           BLACK);

        Paint_ClearWindows(
            LCD_HEIGHT - margin - line_width - 1 - 2 * rounding_radius,
            top_left_corner.y, LCD_HEIGHT - margin - line_width - 1,
            top_left_corner.y + rounding_radius + line_width, BLACK);

        Paint_ClearWindows(margin + line_width,
                           bottom_right_corner.y - rounding_radius -
                               line_width - margin,
                           margin + line_width + 2 * rounding_radius,
                           bottom_right_corner.y, BLACK);

        Paint_ClearWindows(
            LCD_HEIGHT - margin - line_width - 1 - 2 * rounding_radius,
            bottom_right_corner.y - rounding_radius - line_width - margin,
            LCD_HEIGHT - margin - line_width - 1, bottom_right_corner.y, BLACK);

        // The four remaining vertical lines
        Paint_ClearWindows(top_left_corner.x, margin + line_width,
                           top_left_corner.x + rounding_radius + line_width,
                           margin + line_width + rounding_radius, BLACK);

        Paint_ClearWindows(top_left_corner.x,
                           LCD_WIDTH - margin - line_width - 1 -
                               rounding_radius,
                           top_left_corner.x + rounding_radius + line_width,
                           LCD_WIDTH - margin - line_width - 1, BLACK);

        Paint_ClearWindows(
            bottom_right_corner.x - rounding_radius - line_width - 1,
            margin + line_width, bottom_right_corner.x - line_width - 1,
            margin + line_width + rounding_radius, BLACK);

        Paint_ClearWindows(
            bottom_right_corner.x - rounding_radius - line_width - 1,
            LCD_WIDTH - margin - line_width - 1 - rounding_radius,
            bottom_right_corner.x - line_width - 1,
            LCD_WIDTH - margin - line_width - 1, BLACK);
}

static int number_string_length(int number)
{
        if (number >= 1000) {
                return 4;
        } else if (number >= 100) {
                return 3;
        } else if (number >= 10) {
                return 2;
        }
        return 1;
}

static void strReplace(char *str, char *oldWord, char *newWord)
{
        char *pos, temp[1000];
        int index = 0;
        int owlen;

        owlen = strlen(oldWord);

        while ((pos = strstr(str, oldWord)) != NULL) {
                strcpy(temp, str);
                index = pos - str;
                str[index] = '\0';
                strcat(str, newWord);
                strcat(str, temp + index + owlen);
        }
}

void drawGameOver(GameState *state)
{
        drawRoundedBorder(RED);

        char *msg = "Game Over";

        int x_pos = (LCD_HEIGHT - strlen(msg) * FONT_WIDTH) / 2;
        int y_pos = (LCD_WIDTH - FONT_SIZE) / 2;

        Paint_DrawString_EN(x_pos, y_pos, msg, &Font16,
                            BLACK, RED);
}
