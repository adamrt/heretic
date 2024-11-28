#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "bin.h"
#include "event.h"

#define EVENT_FILE_SECTOR (3707)
#define EVENT_FILE_SIZE   (4096000)
#define EVENT_SIZE        (8192)
#define EVENT_COUNT       (500)

// Module private state
static struct {
    event_t events[EVENT_COUNT];
    bool loaded;
} _state;

// Forward declarations
static void _load_events(void);

event_t event_get(int index)
{
    if (!_state.loaded) {
        _load_events();
    }
    return _state.events[index];
}

static void _load_events(void)
{
    file_t event_file = read_file(EVENT_FILE_SECTOR, EVENT_FILE_SIZE);

    for (int i = 0; i < EVENT_COUNT; i++) {
        uint8_t bytes[EVENT_SIZE];
        read_bytes(&event_file, EVENT_SIZE, bytes);

        uint32_t text_offset = (uint32_t)((uint32_t)(bytes[0] & 0xFF)
            | ((uint32_t)(bytes[1] & 0xFF) << 8)
            | ((uint32_t)(bytes[2] & 0xFF) << 16)
            | ((uint32_t)(bytes[3] & 0xFF) << 24));

        _state.events[i].valid = text_offset != 0xF2F2F2F2;

        if (_state.events[i].valid) {
            uint32_t text_size = 8192 - text_offset;
            uint32_t code_size = text_offset - 4;

            _state.events[i].text_size = text_size;
            _state.events[i].code_size = code_size;

            memcpy(_state.events[i].text, bytes + text_offset, text_size);
            memcpy(_state.events[i].code, bytes + 4, code_size);
        }
    }

    free(event_file.data);
    _state.loaded = true;
}
