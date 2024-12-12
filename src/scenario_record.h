#ifndef SCENARIO_RECORD_H_
#define SCENARIO_RECORD_H_

#include "defines.h"
#include "map_record.h"

#define SCENARIO_COUNT (490)

// This may be better named event_record_t, but not 100% yet.
typedef struct {
    int event_id;
    int map_id;
    int entd_id;
    int next_scenario_id;
    weather_e weather;
    time_e time;

    bool valid;
} scenario_record_t;

// Scenario descriptors
typedef struct {
    u16 id;
    const char* name;
} scenario_name_t;

scenario_record_t scenario_get_record(int);

extern scenario_name_t scenario_name_list[];

#endif // SCENARIO_RECORD_H_
