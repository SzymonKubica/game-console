// Copyright (c) 2023 Szymon Kubica
// SPDX-License-Identifier: MIT
#pragma once
#include <cstdarg>
#include <cstdint>
#include <stdio.h>
#include <stdlib.h>

enum class LogLevel : uint8_t {
        LOG_LVL_NONE,  // 0
        LOG_LVL_ERROR, // 1
        LOG_LVL_INFO,  // 2
        LOG_LVL_DEBUG, // 3
        LOG_LVL_TRACE, // 4
};

#ifndef LOG_BUILD_LEVEL
#ifdef DEBUG_BUILD
#define LOG_BUILD_LEVEL LogLevel::LOG_LVL_DEBUG
#else
#define LOG_BUILD_LEVEL LogLevel::LOG_LVL_INFO
#endif
#endif

extern LogLevel log_run_level;

extern const char *log_level_strings[];

// We need to include the Arduino headers to get the Serial monitor printf.
#ifndef EMULATOR
#include "Arduino.h"
#endif

#define LEVEL_ENABLED(level)                                                   \
        (level <= (uint8_t)LOG_BUILD_LEVEL && level <= (uint8_t)log_run_level)

inline void log_message(const char *tag, LogLevel level,
                        const char *function_name, int line, const char *fmt,
                        ...)
{
        if (!LEVEL_ENABLED((uint8_t)level))
                return;

        char buffer[400];
        va_list args;
        // Initializes varargs properly. We need to pass in the last named
        // parameter `fmt` so that the compiler knows where the varargs part
        // starts in memory.
        va_start(args, fmt);
        // This passes the varargs into sprintf so that the format string can be
        // formatted properly.
        vsnprintf(buffer, sizeof(buffer), fmt, args);
        va_end(args);

#ifdef EMULATOR
        printf("%s: [%s] %s:%d: %s\n", tag, log_level_strings[(int)level],
               function_name, line, buffer);
#else
        Serial.printf("%s: [%s] %s:%d: %s\n", tag,
                      log_level_strings[(int)level], function_name, line,
                      buffer);
#endif
}

// We use the __func__ to extract the name of the function from where the log
// is used. This is used instead of the __FUNCTION__ that was previously here.
// That other one is not supported by all compilers and is not a part of the C++
// standard.
#define LOG(tag, level, fmt, ...)                                              \
        log_message(tag, level, __func__, __LINE__, fmt, ##__VA_ARGS__)

#define LOG_INFO(tag, format, ...)                                             \
        LOG(tag, LogLevel::LOG_LVL_INFO, format, ##__VA_ARGS__)
#define LOG_ERROR(tag, format, ...)                                            \
        LOG(tag, LogLevel::LOG_LVL_ERROR, format, ##__VA_ARGS__)
#define LOG_DEBUG(tag, format, ...)                                            \
        LOG(tag, LogLevel::LOG_LVL_DEBUG, format, ##__VA_ARGS__)
#define LOG_TRACE(tag, format, ...)                                            \
        LOG(tag, LogLevel::LOG_LVL_TRACE, format, ##__VA_ARGS__)
