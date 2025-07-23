#pragma once

#include "assert.h"
#include "defines.h"
#include "map_record.h"
#include "vm_event.h"

enum {
    SCENARIO_COUNT = 491,
    SCENARIO_SIZE = 24
};

typedef struct {
    int event_id;
    int map_id;
    weather_e weather;
    time_e time;
    int entd_id;
    int next_scenario_id;
    u8 data[SCENARIO_SIZE];
} scenario_t;

scenario_t scenario_get_scenario(int);

static_assert(VM_EVENT_COUNT == SCENARIO_COUNT, "Event/scenario count mismatch");
