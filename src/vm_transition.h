#pragma once

#include <stdbool.h>

#include "defines.h"
#include "vm.h"
#include "vm_opcode.h"

enum {
    VM_TRANSITION_MAX = 64,
};

// Transition struct to simplify interpolation of values.
typedef struct {
    f32 start;
    f32 end;
    f32 frame_total;
    f32 frame_current;
    void* target;
    opcode_e opcode;
} transition_t;

typedef struct {
    transition_t transitions[VM_TRANSITION_MAX];
    size_t transaction_count;
} transition_manager_t;

void vm_transition_reset(void);
void vm_transition_update(void);
void vm_transition_add(opcode_e, void*, f32, f32, f32);
bool vm_transition_has_active(waittype_e);
