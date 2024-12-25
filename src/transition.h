#pragma once

#include <stdbool.h>

#include "defines.h"
#include "opcode.h"
#include "vm.h"

#define TRANSITION_MAX 64

// Transition struct to simplify interpolation of values.
typedef struct {
    f32 start;
    f32 end;
    f32 frame_total;
    f32 frame_current;
    void* target;
    opcode_id_t opcode_id;
} transition_t;

typedef struct {
    transition_t transitions[TRANSITION_MAX];
    size_t transaction_count;
} transition_manager_t;

void transition_update(void);
void transition_add(opcode_id_t, void*, f32, f32, f32);
bool transition_active(void);
bool transition_has_active(waittype_e);
