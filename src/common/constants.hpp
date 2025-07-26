/* Definitions of platform-specific constants that are commonly used by the games.*/
#pragma once

#define FONT_SIZE 16
#define HEADING_FONT_SIZE 24
#define HEADING_FONT_WIDTH 17

// The font on the emulator is not pixel-accurate the same as what we
// have on the actual hardware. Because of this we need this conditional
// constant definition.
#ifdef EMULATOR
#define FONT_WIDTH 10
#endif
#ifndef EMULATOR
#define FONT_WIDTH 11
#endif

#define SCREEN_BORDER_WIDTH 3

/* Constants below control time intervals between input polling */
#define INPUT_POLLING_DELAY 50
#define MOVE_REGISTERED_DELAY 150


