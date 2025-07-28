#pragma once

/**
 * A copy of the color enum from the library to avoid having an explicit
 * dependency on it in the generic user interface code.
 */
#include <cstring>
typedef enum Color {
        White = 0xFFFF,
        Black = 0x0000,
        Blue = 0x001F,
        BRed = 0XF81F,
        GRed = 0XFFE0,
        Gblue = 0X07FF,
        Red = 0xF800,
        Magenta = 0xF81F,
        Green = 0x07E0,
        Cyan = 0x7FFF,
        Yellow = 0xFFE0,
        Brown = 0XBC40,
        BRRed = 0XFC07,
        Gray = 0X8430,
        DarkBlue = 0X01CF,
        LightBlue = 0X7D7C,
        GrayBlue = 0X5458,
        LightGreen = 0X841F,
        LGray = 0XC618,
        LGrayBlue = 0XA651,
        LBBlue = 0X2B12,
} Color;

const char *map_color(Color color);
