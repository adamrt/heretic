#pragma once

#include "defines.h"
#include "map_record.h"

#define SCENARIO_COUNT (491)
#define SCENARIO_SIZE  (24)

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
