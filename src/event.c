#include <string.h>

#include "bin.h"
#include "event.h"
#include "game.h"

void load_events(void)
{
    file_t event_file = read_file(EVENT_FILE_SECTOR, EVENT_FILE_SIZE);
    event_t* events = calloc(EVENT_COUNT, sizeof(event_t));

    for (int i = 0; i < EVENT_COUNT; i++) {
        uint8_t bytes[EVENT_SIZE];
        read_bytes(&event_file, EVENT_SIZE, bytes);

        uint32_t text_offset = (uint32_t)((uint32_t)(bytes[0] & 0xFF)
            | ((uint32_t)(bytes[1] & 0xFF) << 8)
            | ((uint32_t)(bytes[2] & 0xFF) << 16)
            | ((uint32_t)(bytes[3] & 0xFF) << 24));

        events[i].valid = text_offset != 0xF2F2F2F2;

        if (events[i].valid) {
            int text_size = 8192 - text_offset;
            int code_size = text_offset - 4;

            memcpy(events[i].text, bytes + text_offset, text_size);
            memcpy(events[i].code, bytes + 4, code_size);
        }
    }

    g.fft.events = events;

    free(event_file.data);
}
