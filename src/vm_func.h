#pragma once

#include "vm_instruction.h"

// Function pointer type for opcode functions
typedef void (*opcode_fn_t)(const instruction_t*);

void vm_func_display_message(const instruction_t*);
void vm_func_camera(const instruction_t*);
void vm_func_wait_for_instruction(const instruction_t*);
void vm_func_warp_unit(const instruction_t*);
