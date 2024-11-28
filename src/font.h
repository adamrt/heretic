#ifndef FONT_H_
#define FONT_H_

#include <stdint.h>

#define FONT_CHAR_COUNT (2200)

typedef struct {
    uint16_t id;
    const char* data;
} font_char_t;

const char* font_get_char(uint16_t id);

#endif // FONT_H_
