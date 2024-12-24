#pragma once

#include "event.h"
#include "opcode.h"
#include "vm_func.h"

// Virtual machine state
typedef struct {
    opcode_fn_t handlers[OPCODE_ID_MAX];
    const event_t* current_event;
    size_t current_instruction;
    bool is_executing;
} vm_t;

void vm_init(void);
void vm_execute_event(const event_t* event);
void vm_update(void);
void vm_reset(void);
int vm_get_current_instruction(void);
