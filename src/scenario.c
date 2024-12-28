#include <string.h>

#include "filesystem.h"
#include "scenario.h"
#include "span.h"
#include "util.h"

#define SCENARIO_OFFSET (0x10938)

static scenario_t read_scenario(span_t*);

scenario_t scenario_get_scenario(int id) {
    ASSERT(id < SCENARIO_COUNT, "Scenario id %d out of bounds", id);
    span_t file = filesystem_read_file(F_EVENT__ATTACK_OUT);
    span_t span = {
        .data = file.data + SCENARIO_OFFSET + (id * SCENARIO_SIZE),
        .size = SCENARIO_SIZE,
    };
    file.offset = SCENARIO_OFFSET + (id * SCENARIO_SIZE);
    scenario_t scenario = read_scenario(&span);
    return scenario;
}

static scenario_t read_scenario(span_t* span) {
    scenario_t scenario = {};
    scenario.event_id = span_readat_u16(span, 0);
    scenario.map_id = span_readat_u8(span, 2);
    scenario.weather = span_readat_u8(span, 3);
    scenario.time = span_readat_u8(span, 4);
    scenario.entd_id = span_readat_u16(span, 7);
    scenario.next_scenario_id = span_readat_u16(span, 18);
    memcpy(&scenario.data, &span->data[0], SCENARIO_SIZE);
    return scenario;
}
