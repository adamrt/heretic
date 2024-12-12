#pragma once

#include <stdbool.h>

#include "event.h"
#include "gfx.h"
#include "map.h"

typedef enum {
    MODE_MAP,
    MODE_SCENARIO,
} mode_e;

typedef struct {
    mode_e mode;

    map_t* map;
    model_t models[10];
    int model_count;

    bool center_model;
    int current_scenario;
    int current_map;

    message_t* messages;
    instruction_t* instructions;
    int message_count;
    int instruction_count;
} scene_t;

void scene_init(void);
void scene_shutdown(void);
void scene_update(void);
void scene_render(void);

// Temporary
scene_t* scene_get_internals(void);

void scene_load_map(int num, map_state_t state);
void scene_load_scenario(int num);

message_t* scene_get_messages(void);
instruction_t* scene_get_instructions(void);
int scene_get_message_count(void);
int scene_get_instruction_count(void);

void scene_prev(void);
void scene_next(void);
