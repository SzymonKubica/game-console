#pragma once

/**
 * Pin Mode enum copied from Arduino libs for compatibility.
 * This allows to reuse the same enum variants in the C++ code it is not
 * possible to import them.
 */
typedef enum {
        INPUT = 0x0,
        OUTPUT = 0x1,
        INPUT_PULLUP = 0x2,
        INPUT_PULLDOWN = 0x3,
        OUTPUT_OPENDRAIN = 0x4,
} PinMode;
