#include <string.h>

#include "defines.h"
#include "event.h"
#include "instruction.h"
#include "memory.h"
#include "message.h"
#include "span.h"

event_t read_event(span_t* span) {
    event_t event = { 0 };

    u32 text_offset = span_read_u32(span);
    bool valid = text_offset != 0xF2F2F2F2;
    if (!valid) {
        return event;
    }

    usize text_size = EVENT_SIZE - text_offset;
    usize code_size = text_offset - 4;

    event.valid = valid;
    memcpy(event.data, span->data, EVENT_SIZE);

    span_t msgspan = { .data = event.data + text_offset, .offset = 0 };
    event.messages = memory_allocate(MESSAGES_LEN);
    event.messages_len = read_messages(&msgspan, text_size, event.messages);

    span_t instspan = { .data = event.data + 4, .offset = 0 };
    event.instructions = memory_allocate(INSTRUCTION_MAX * sizeof(instruction_t));
    event.instruction_count = read_instructions(&instspan, code_size, event.instructions);

    return event;
}
