#pragma once

#include "instruction.h"

// Function pointer type for opcode functions
typedef void (*opcode_fn_t)(const instruction_t*);

void fn_10_displaymessage(const instruction_t*);
void fn_19_camera(const instruction_t*);
