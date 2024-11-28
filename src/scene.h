#ifndef SCENE_H_
#define SCENE_H_

#include <stdbool.h>

#include "gfx.h"
#include "map.h"

typedef enum {
    MODE_MAP,
    MODE_SCENARIO,
} mode_e;

typedef struct {
    mode_e mode;

    map_t* map;
    model_t model;
    bool center_model;
    int current_scenario;
    int current_map;
} scene_t;

void scene_init(void);
void scene_update(void);
void scene_shutdown(void);

// Temporary
scene_t* scene_get_internals(void);

void scene_load_map(int num, map_state_t state);
void scene_load_scenario(int num);

void scene_prev(void);
void scene_next(void);

#endif // SCENE_H_
