#include "color.hpp"

const char *map_color(Color color)
{
        switch (color) {

        case White:
                return "White";
        case Black:
                return "Black";
        case Blue:
                return "Blue";
        case BRed:
                return "BRed";
        case GRed:
                return "GRed";
        case Gblue:
                return "Gblue";
        case Red:
                return "Red";
        case Green:
                return "Green";
        case Cyan:
                return "Cyan";
        case Brown:
                return "Brown";
        case BRRed:
                return "BRRed";
        case Gray:
                return "Gray";
        case DarkBlue:
                return "DarkBlue";
        case LightBlue:
                return "LightBlue";
        case GrayBlue:
                return "GrayBlue";
        case LightGreen:
                return "LightGreen";
        case LGray:
                return "LGray";
        case LGrayBlue:
                return "LGrayBlue";
        case LBBlue:
                return "LBBlue";
        default:
                return "UNKNOWN";
        };
}
