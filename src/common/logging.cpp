// Copyright (c) 2023 Szymon Kubica
// SPDX-License-Identifier: MIT
#include "logging.hpp"

unsigned char log_run_level = LOG_LVL_DEBUG;

const char *log_level_strings[] = {
    [LOG_LVL_NONE] = "NONE",       [LOG_LVL_ERROR] = "ERROR",
    [LOG_LVL_INFO] = "INFO",       [LOG_LVL_DEBUG] = "DEBUG",
    [LOG_LVL_VERBOSE] = "VERBOSE",
};

