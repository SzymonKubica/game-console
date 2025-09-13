// Copyright (c) 2023 Szymon Kubica
// SPDX-License-Identifier: MIT
#include "logging.hpp"

LogLevel log_run_level = LogLevel::LOG_LVL_DEBUG;

const char *log_level_strings[] = {
    [(uint8_t)LogLevel::LOG_LVL_NONE] = "NONE",
    [(uint8_t)LogLevel::LOG_LVL_ERROR] = "ERROR",
    [(uint8_t)LogLevel::LOG_LVL_INFO] = "INFO",
    [(uint8_t)LogLevel::LOG_LVL_DEBUG] = "DEBUG",
    [(uint8_t)LogLevel::LOG_LVL_TRACE] = "TRACE",
};
