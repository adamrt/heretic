#pragma once

#include <stdio.h>
#include <stdlib.h>

#include "defines.h"

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

// Bytes to kilobytes and megabytes.
#define BYTES_TO_KB(x) ((f64)(x) / 1024.0)
#define BYTES_TO_MB(x) ((f64)(x) / (1024.0 * 1024.0))

// Usage:
//   ASSERT(condition, "Simple message");
//   ASSERT(condition, "Formatted message: %d, %s", value1, value2);
#define ASSERT(cond, ...)                                       \
    do {                                                        \
        if (!(cond)) {                                          \
            fprintf(stderr, "Assertion failed: " __VA_ARGS__);  \
            fprintf(stderr, " at %s:%d\n", __FILE__, __LINE__); \
            exit(EXIT_FAILURE);                                 \
        }                                                       \
    } while (0)
