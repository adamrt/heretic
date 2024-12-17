#include "instruction.h"
#include "opcode.h"
#include "util.h"

usize read_instructions(span_t* span, usize spansize, instruction_t* out_instructions) {
    usize count = 0;
    while (span->offset < spansize) {
        opcode_id_t id = (opcode_id_t)span_read_u8(span);
        opcode_desc_t desc = opcode_desc_list[id];
        instruction_t instruction = {
            .opcode = id
        };

        for (int i = 0; i < desc.param_count; i++) {
            param_t param = { 0 };
            if (desc.param_sizes[i] == PARAM_TYPE_U16) {
                param.type = PARAM_TYPE_U16;
                param.value.u16 = span_read_u16(span);
            } else if (desc.param_sizes[i] == PARAM_TYPE_U8) {
                param.type = PARAM_TYPE_U8;
                param.value.u8 = span_read_u8(span);
            } else {
                ASSERT(false, "Unknown param type %d %s", desc.param_sizes[i], desc.name);
            }
            instruction.params[i] = param;
        }
        out_instructions[count++] = instruction;
        ASSERT(count < INSTRUCTION_MAX, "Instruction count exceeded");
    }
    return count;
}
