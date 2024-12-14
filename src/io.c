#include <math.h>
#include <string.h>

#include "event.h"
#include "io.h"
#include "memory.h"
#include "scenario.h"
#include "util.h"

#define SECTOR_HEADER_SIZE (24)
#define SECTOR_SIZE        (2048)
#define SECTOR_SIZE_RAW    (2352)

// Individual file lengths
#define ATTACK_OUT_LEN             (125956)
#define ATTACK_OUT_SCENARIO_OFFSET (0x10938)
#define TEST_EVT_LEN               (4096000)

static struct {
    FILE* bin;

    u8* test_evt;
    u8* attack_out;

    scenario_t* scenarios;
    event_t* events;
} _state;

void io_init(void)
{
    _state.bin = fopen("../fft.bin", "rb");
    ASSERT(_state.bin != NULL, "Failed to open fft.bin");

    // Allocate for all io resources
    _state.attack_out = memory_allocate(ATTACK_OUT_LEN);
    _state.test_evt = memory_allocate(TEST_EVT_LEN);
    _state.scenarios = memory_allocate(SCENARIO_COUNT * sizeof(scenario_t));
    _state.events = memory_allocate(EVENT_COUNT * sizeof(event_t));

    // Read all files
    io_read_file(2448, ATTACK_OUT_LEN, _state.attack_out);
    io_read_file(3707, TEST_EVT_LEN, _state.test_evt);

    // Read all scenarios and events
    for (usize i = 0; i < SCENARIO_COUNT; i++) {
        buffer_t buf = {
            .data = _state.attack_out + ATTACK_OUT_SCENARIO_OFFSET + (i * SCENARIO_SIZE),
            .offset = 0,
        };
        _state.scenarios[i] = read_scenario(&buf);
    }

    for (usize i = 0; i < EVENT_COUNT; i++) {
        buffer_t buf = {
            .data = _state.test_evt + (i * EVENT_SIZE),
            .offset = 0,
        };
        _state.events[i] = read_event(&buf);
    }
}

void io_shutdown(void)
{
    fclose(_state.bin);
    memory_free(_state.test_evt);
    memory_free(_state.attack_out);
    memory_free(_state.scenarios);

    for (usize i = 0; i < EVENT_COUNT; i++) {
        event_t* event = &_state.events[i];
        for (int j = 0; j < event->message_count; j++) {
            memory_free(event->messages[j].cstr);
        }
        memory_free(event->messages);
        event->message_count = 0;
    }
    memory_free(_state.events);
}

void io_read_file(usize sector_num, usize size, u8* out_bytes)
{
    usize offset = 0;
    usize occupied_sectors = ceil(size / (f64)SECTOR_SIZE);

    for (usize i = 0; i < occupied_sectors; i++) {
        usize seek_to = ((sector_num + i) * SECTOR_SIZE_RAW) + SECTOR_HEADER_SIZE;
        usize sn = fseek(_state.bin, seek_to, SEEK_SET);
        ASSERT(sn == 0, "Failed to seek to sector");

        u8 sector[SECTOR_SIZE];
        usize rn = fread(sector, sizeof(u8), SECTOR_SIZE, _state.bin);
        ASSERT(rn == SECTOR_SIZE, "Failed to read correct number of bytes from sector");

        usize remaining_size = size - offset;
        usize bytes_to_copy = (remaining_size < SECTOR_SIZE) ? remaining_size : SECTOR_SIZE;

        memcpy(out_bytes + offset, sector, bytes_to_copy);
        offset += bytes_to_copy;
    }

    return;
}

// Getters for resources
event_t io_read_event(int id)
{
    ASSERT(id < EVENT_COUNT, "Event id %d out of bounds", id);
    return _state.events[id];
}

scenario_t io_read_scenario(int id)
{
    ASSERT(id < SCENARIO_COUNT, "Scenario id %d out of bounds", id);
    return _state.scenarios[id];
}

// Return preloaded files
file_t io_file_test_evt(void) { return (file_t) { _state.test_evt, TEST_EVT_LEN }; }
file_t io_file_attack_out(void) { return (file_t) { _state.attack_out, ATTACK_OUT_LEN }; }
