#include <string.h>

#include "defines.h"
#include "event.h"
#include "instruction.h"
#include "io.h"
#include "memory.h"
#include "message.h"
#include "span.h"
#include "util.h"

static struct {
    event_t* events;
} _state;

static event_t read_event(span_t*);

void event_init(void) {
    // Allocate for all io resources
    _state.events = memory_allocate(EVENT_COUNT * sizeof(event_t));

    span_t file = io_file_test_evt();
    for (usize i = 0; i < EVENT_COUNT; i++) {
        span_t span = {
            .data = file.data + (i * EVENT_SIZE),
            .size = EVENT_SIZE,
        };
        _state.events[i] = read_event(&span);
    }
}

void event_shutdown(void) {
    for (usize i = 0; i < EVENT_COUNT; i++) {
        memory_free(_state.events[i].messages);
        memory_free(_state.events[i].instructions);
    }
    memory_free(_state.events);
}

event_t event_get_event(int id) {
    ASSERT(id < EVENT_COUNT, "Event id %d out of bounds", id);
    return _state.events[id];
}

static event_t read_event(span_t* span) {
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
