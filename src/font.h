#ifndef FONT_H_
#define FONT_H_

#include <stdint.h>

#include "defines.h"

#define FONT_CHAR_COUNT (2200)

typedef struct {
    u16 id;
    const char* data;
} font_char_t;

const char* font_get_char(u16 id);

#endif // FONT_H_
