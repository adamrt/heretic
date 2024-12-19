#pragma once

#include <stdbool.h>

#include "defines.h"

#define TRANSITION_MAX 64

// Transition struct to simplify interpolation of values.
typedef struct {
    f32 start;
    f32 end;
    f32 frame_total;
    f32 frame_current;
    void* target;
} transition_t;

typedef struct {
    transition_t transitions[TRANSITION_MAX];
    size_t transaction_count;
} transition_manager_t;

void transition_update(void);
void transition_add(void*, f32, f32, f32);
bool transition_active(void);
