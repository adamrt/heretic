#pragma once

#include <stdbool.h>

#include "event.h"
#include "map.h"
#include "map_record.h"

typedef enum {
    MODE_EVENT,
    MODE_MAP,
} mode_e;

typedef struct {
    mode_e mode;
    map_t* map;
    int current_scenario_id;
    int current_map;
    event_t event;
} scene_t;

void scene_init(void);
void scene_shutdown(void);
void scene_render(void);

// Temporary
scene_t* scene_get_internals(void);

void scene_load_map(int, map_state_t);
void scene_load_scenario(int);
void scene_set_map_rotation(f32);

event_t scene_get_event(void);

void scene_prev(void);
void scene_next(void);
