#ifndef UTIL_H_
#define UTIL_H_

#include <stdio.h>
#include <stdlib.h>

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

#endif // UTIL_H_
