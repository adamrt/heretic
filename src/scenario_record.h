#ifndef SCENARIO_RECORD_H_
#define SCENARIO_RECORD_H_

#include "map_record.h"

#define SCENARIO_COUNT (490)

typedef struct {
    int id;
    int map_id;
    weather_e weather;
    time_e time;
    int entd_id;
    int next_scenario_id;
} scenario_record_t;

// Scenario descriptors
typedef struct {
    uint16_t id;
    const char* name;
} scenario_desc_t;

scenario_record_t scenario_get_record(int);

extern scenario_desc_t scenario_record_list[];

#endif // SCENARIO_RECORD_H_
