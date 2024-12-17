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

    memcpy(event.data, span->data, EVENT_SIZE);

    usize text_size = EVENT_SIZE - text_offset;
    span_t text_span = {
        .data = event.data + text_offset,
        .size = text_size,
    };
    event.messages = memory_allocate(MESSAGES_LEN);
    event.messages_len = read_messages(&text_span, event.messages);

    usize code_size = text_offset - 4;
    span_t code_span = {
        .data = event.data + 4,
        .size = code_size,
    };
    event.instructions = memory_allocate(INSTRUCTION_MAX * sizeof(instruction_t));
    event.instruction_count = read_instructions(&code_span, event.instructions);

    event.valid = valid;
    return event;
}
