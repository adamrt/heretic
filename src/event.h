#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "defines.h"
#include "instruction.h"
#include "span.h"

#define EVENT_SIZE  (8192)
#define EVENT_COUNT (500)

// An event is a list of text and instructions for a particular scenario.
//
// Events are alway 8192 (0x2000) bytes long. There are 3 components.
// - text_offset: First 4 bytes is a pointer to the to the text_section.
//   - If the offset is 0xF2F2F2F2, then the event should be skipped.
//     These are battle setup events and other non map events.
// - code_section: Bytes 5 to text_offset is the code section.
// - text_section: Bytes text_offset thru 8192 is the text section.
typedef struct {
    char* messages;
    int messages_len;

    instruction_t* instructions;
    int instruction_count;

    u8 data[EVENT_SIZE];
    bool valid;
} event_t;

event_t read_event(span_t*);
