#ifndef EVENT_H_
#define EVENT_H_

#include <stdbool.h>
#include <stdint.h>

typedef struct {
    int text_count;
    int code_count;

    uint8_t text[8192];
    uint8_t code[8192];
    bool valid;
} event_t;

event_t event_get(int);

#endif // EVENT_H_
