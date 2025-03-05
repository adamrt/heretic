#pragma once

#include <stdbool.h>

#include "event.h"
#include "map.h"
#include "map_record.h"
#include "model.h"
#include "sprite.h"

typedef enum {
    MODE_EVENT,
    MODE_MAP,
} mode_e;

typedef struct {
    mode_e mode;

    model_t model;
    map_t* map;
    sprite3d_t sprite3ds[100];
    sprite2d_t sprite2ds[100];

    int current_scenario_id;
    int current_map;

    event_t event;
} scene_t;

void scene_init(void);
void scene_shutdown(void);
void scene_update(void);
void scene_render(void);

// Temporary
scene_t* scene_get_internals(void);

void scene_load_map(int, map_state_t);
void scene_load_scenario(int);
void scene_set_map_rotation(f32);

event_t scene_get_event(void);

void scene_prev(void);
void scene_next(void);
