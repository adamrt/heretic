#pragma once

#include "assert.h"
#include "defines.h"
#include "event.h"
#include "map_record.h"

constexpr int SCENARIO_COUNT = 491;
constexpr int SCENARIO_SIZE = 24;

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

static_assert(EVENT_COUNT == SCENARIO_COUNT, "Event/scenario count mismatch");
