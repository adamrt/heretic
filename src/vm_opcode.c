#include "vm_opcode.h"

// clang-format off
const opcode_desc_t opcode_desc_list[] = {
#define X(id, value, name, params, param_count) [id] = { value, name,  params, param_count },
    OPCODE_LIST
#undef X
};
// clang-format on
