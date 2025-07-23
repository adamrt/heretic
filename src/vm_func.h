#pragma once

#include "vm_instruction.h"

// Function pointer type for opcode functions
typedef void (*opcode_fn_t)(const instruction_t*);

void fn_display_message(const instruction_t*);
void fn_camera(const instruction_t*);
void fn_wait_for_instruction(const instruction_t*);
void fn_warp_unit(const instruction_t*);
