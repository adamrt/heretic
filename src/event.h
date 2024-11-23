#ifndef EVENT_H_
#define EVENT_H_

#include <stdbool.h>
#include <stdint.h>

#define EVENT_FILE_SECTOR (3707)
#define EVENT_FILE_SIZE   (4096000)
#define EVENT_SIZE        (8192)
#define EVENT_COUNT       (500)

typedef struct {
    int text_count;
    int code_count;

    uint8_t text[8192];
    uint8_t code[8192];
    bool valid;
} event_t;

void load_events(void);

#endif // EVENT_H_
