#ifndef SCENE_H_
#define SCENE_H_

#include <stdbool.h>

#include "fft.h"
#include "gfx.h"

typedef struct {
    map_t* map;
    model_t model;
    bool center_model;
    int current_scenario;
    int current_map;
} scene_t;

void game_map_load(int num, map_state_t state);
void map_unload(void);
void game_scenario_load(int num);
void map_prev(void);
void map_next(void);

#endif // SCENE_H_
