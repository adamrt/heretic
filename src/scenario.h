#ifndef SCENARIO_H_
#define SCENARIO_H_

#include "map_record.h"

#define SCENARIO_COUNT (490)

typedef struct {
    int id;
    int map_id;
    weather_e weather;
    time_e time;

    int entd_id;
    int next_scenario_id;
} scenario_t;

// Scenario and map descriptors
typedef struct {
    uint16_t id;
    const char* name;
} scenario_desc_t;

void load_scenarios(void);

extern scenario_desc_t scenario_list[];

#endif // SCENARIO_H_
