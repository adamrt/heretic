#include <string.h>

#include "event.h"
#include "io.h"
#include "memory.h"
#include "scenario.h"
#include "span.h"
#include "util.h"

#define SCENARIO_OFFSET (0x10938)

static struct {
    scenario_t* scenarios;
} _state;

static scenario_t read_scenario(span_t*);

void scenario_init(void) {
    _state.scenarios = memory_allocate(SCENARIO_COUNT * sizeof(scenario_t));
    span_t file = io_file_attack_out();
    for (usize i = 0; i < SCENARIO_COUNT; i++) {
        span_t span = {
            .data = file.data + SCENARIO_OFFSET + (i * SCENARIO_SIZE),
            .size = SCENARIO_SIZE,
        };
        _state.scenarios[i] = read_scenario(&span);
    }
}

void scenario_shutdown(void) {
    memory_free(_state.scenarios);
}

scenario_t scenario_get_scenario(int id) {
    ASSERT(id < SCENARIO_COUNT, "Scenario id %d out of bounds", id);
    return _state.scenarios[id];
}

static scenario_t read_scenario(span_t* span) {
    scenario_t scenario = { 0 };
    scenario.event_id = span_readat_u16(span, 0);
    scenario.map_id = span_readat_u8(span, 2);
    scenario.weather = span_readat_u8(span, 3);
    scenario.time = span_readat_u8(span, 4);
    scenario.entd_id = span_readat_u16(span, 7);
    scenario.next_scenario_id = span_readat_u16(span, 18);
    memcpy(&scenario.data, &span->data[0], SCENARIO_SIZE);
    return scenario;
}
