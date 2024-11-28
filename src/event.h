#ifndef EVENT_H_
#define EVENT_H_

#include <stdbool.h>
#include <stdint.h>

#define EVENT_MAX_TEXT_SIZE (8141)
#define EVENT_MAX_CODE_SIZE (3647)

// An event is a list of text and instructions for a particular scenario.
//
// Events are alway 8192 (0x2000) bytes long. There are 3 components.
// - text_offset: First 4 bytes is a pointer to the to the text_section.
//   - If the offset is 0xF2F2F2F2, then the event should be skipped.
//     These are battle setup events and other non map events.
// - code_section: Bytes 5 to text_offset is the code section.
// - text_section: Bytes text_offset thru 8192 is the text section.
typedef struct {
    uint8_t text[EVENT_MAX_TEXT_SIZE];
    uint8_t code[EVENT_MAX_CODE_SIZE];
    int text_size;
    int code_size;
    bool valid;
} event_t;

event_t event_get(int);

#endif // EVENT_H_
