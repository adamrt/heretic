#ifndef BIN_H_
#define BIN_H_

#include <stdint.h>
#include <stdio.h>

#include "model.h"

typedef enum {
    TIME_DAY = 0x0,
    TIME_NIGHT = 0x1,
} time_e;

typedef enum {
    WEATHER_NONE = 0x0,
    WEATHER_NONE_ALT = 0x1,
    WEATHER_NORMAL = 0x2,
    WEATHER_STRONG = 0x3,
    WEATHER_VERY_STRONG = 0x4,
} weather_e;

// Each resource is a file that contains a single type of data (mesh, texture).
// Each resource is related to a specific time, weather, and layout.
typedef struct {
    time_e time;
    weather_e weather;
    int layout;
} resource_key_t;

typedef struct {
    int id;
    int map_id;
    weather_e weather;
    time_e time;

    int entd_id;
    int next_scenario_id;
} scenario_t;

typedef struct {
    scenario_t scenarios[500];
    int count;
} scenarios_t;

model_t read_map(int num, resource_key_t requested_key);
scenarios_t read_scenarios(void);

void time_str(time_e, char[static 8]);
void weather_str(weather_e, char[static 12]);

// Scenario and map descriptors
typedef struct {
    uint16_t id;
    const char* name;
} scenario_desc_t;

typedef struct {
    uint8_t id;
    uint16_t sector;
    bool valid;
    const char* name;
} map_desc_t;

extern scenario_desc_t scenario_list[];
extern map_desc_t map_list[];

#endif // BIN_H_
