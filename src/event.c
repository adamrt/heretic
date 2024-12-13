#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bin.h"
#include "defines.h"
#include "event.h"
#include "font.h"
#include "io.h"
#include "util.h"

#define EVENT_SIZE (8192)

event_t event_get_event(int event_id)
{
    buffer_t f = read_file_test_evt();
    f.offset = event_id * EVENT_SIZE;

    u8 bytes[EVENT_SIZE];
    read_bytes(&f, sizeof(bytes), bytes);

    u32 text_offset = (u32)((u32)(bytes[0] & 0xFF)
        | ((u32)(bytes[1] & 0xFF) << 8)
        | ((u32)(bytes[2] & 0xFF) << 16)
        | ((u32)(bytes[3] & 0xFF) << 24));

    if (text_offset == 0xF2F2F2F2) {
        return (event_t) { 0 };
    }

    event_t event = {
        .text_size = 8192 - text_offset,
        .code_size = text_offset - 4,
        .valid = true,
    };

    ASSERT(event.text_size <= EVENT_TEXT_SIZE_MAX, "Event text size exceeded");
    ASSERT(event.code_size <= EVENT_CODE_SIZE_MAX, "Event code size exceeded");

    memcpy(event.text, bytes + text_offset, event.text_size);
    memcpy(event.code, bytes + 4, event.code_size);

    return event;
}

message_t* event_get_messages(event_t event, int* count)
{
    message_t* messages = calloc(EVENT_MESSAGE_MAX, sizeof(message_t));
    ASSERT(messages != NULL, "Failed to allocate memory for messages");

    u8 delimiter = 0xFE;
    char event_text[EVENT_TEXT_SIZE_MAX * 2]; // Adjust size as needed
    usize event_text_len = 0;

    usize i = 0;
    while (i < event.text_size) {
        u8 byte = event.text[i];

        // These are special characters. We need to handle them differently.
        // https://ffhacktics.com/wiki/Text_Format#Special_Characters
        switch (byte) {
        case 0xFE:
            // This is the message delimiter.
            event_text[event_text_len++] = (char)delimiter;
            i++;
            break;
        case 0xE0: {
            // Character name stored somewhere else. Hard coding for now.
            const char* name = "Ramza";
            usize len = strlen(name);
            memcpy(&event_text[event_text_len], name, len);
            event_text_len += len;
            i++;
            break;
        }
        case 0xE2: {
            i++;
            if (i >= event.text_size)
                break;
            u8 delay = event.text[i];
            char buffer[32];
            int len = snprintf(buffer, sizeof(buffer), "{Delay: %d}", (int)delay);
            memcpy(&event_text[event_text_len], buffer, len);
            event_text_len += len;
            i++;
            break;
        }
        case 0xE3: {
            i++;
            if (i >= event.text_size)
                break;
            u8 color = event.text[i];
            char buffer[32];
            int len = snprintf(buffer, sizeof(buffer), "{Color: %d}", (int)color);
            memcpy(&event_text[event_text_len], buffer, len);
            event_text_len += len;
            i++;
            break;
        }
        case 0xF0:
        case 0xF1:
        case 0xF2:
        case 0xF3: {
            // This is a jump to another point in the text section.
            // The next 2 bytes are the jump location and how many bytes to read.
            // https://gomtuu.org/fft/trans/compression/
            i++;
            if (i + 1 >= event.text_size)
                break;
            u8 second_byte = event.text[i];
            u8 third_byte = event.text[i + 1];
            (void)second_byte; // Unused
            (void)third_byte;  // Unused
            const char* text_jump = "{TextJump}";
            usize len = strlen(text_jump);
            memcpy(&event_text[event_text_len], text_jump, len);
            event_text_len += len;
            i += 2;
            break;
        }
        case 0xF8:
            event_text[event_text_len++] = '\n';
            i++;
            break;
        case 0xFA:
            // This one is not in the list but it is very common between words.
            // It works well as a space though.
            event_text[event_text_len++] = ' ';
            i++;
            break;
        case 0xFF: {
            const char* close_str = "{Close}";
            usize len = strlen(close_str);
            memcpy(&event_text[event_text_len], close_str, len);
            event_text_len += len;
            i++;
            break;
        }
        default: {
            if (byte > 0xCF) {
                /* Two-byte character */
                if (i + 1 >= event.text_size) {
                    /* Can't read second byte */
                    i++;
                    break;
                }
                u8 second_byte = event.text[i + 1];
                u16 combined = (second_byte | (byte << 8));
                const char* font_str = font_get_char(combined);
                if (font_str != NULL) {
                    usize len = strlen(font_str);
                    memcpy(&event_text[event_text_len], font_str, len);
                    event_text_len += len;
                    i += 2;
                } else {
                    /* Unknown character */
                    char buffer[64];
                    int len = snprintf(buffer, sizeof(buffer), "{Unknown: 0x%X & 0x%X}", byte, second_byte);
                    memcpy(&event_text[event_text_len], buffer, len);
                    event_text_len += len;
                    i++;
                }
            } else {
                /* Single-byte character */
                const char* font_str = font_get_char(byte);
                if (font_str != NULL) {
                    usize len = strlen(font_str);
                    memcpy(&event_text[event_text_len], font_str, len);
                    event_text_len += len;
                    i++;
                } else {
                    /* Unknown character */
                    event_text[event_text_len++] = '?';
                    i++;
                }
            }
            break;
        }
        }
    }

    event_text[event_text_len] = '\0';

    // Split the text into messages

    const char* start = event_text;
    char* end = event_text + event_text_len;

    while (start < end && *count < EVENT_MESSAGE_MAX) {
        /* Find next delimiter */
        char* delimiter_pos = memchr(start, (char)delimiter, end - start);
        usize len;
        if (delimiter_pos != NULL) {
            len = delimiter_pos - start;
        } else {
            len = end - start;
        }

        /* Allocate memory for the message */
        message_t message = {
            .len = len + 1, // +1 for null terminator
        };
        message.cstr = calloc(1, message.len);
        ASSERT(message.cstr != NULL, "Failed to allocate memory for message");

        memcpy(message.cstr, start, len);
        message.cstr[len] = '\0'; // not using message.len

        messages[(*count)++] = message;

        if (delimiter_pos != NULL) {
            start = delimiter_pos + 1;
        } else {
            break;
        }
    }
    return messages;
}

instruction_t* event_get_instructions(event_t event, int* count)
{
    instruction_t* instructions = calloc(EVENT_INSTRUCTION_MAX, sizeof(instruction_t));
    ASSERT(instructions != NULL, "Failed to allocate memory for instructions");

    usize code_idx = 0;
    while (code_idx < event.code_size) {
        u8 code = event.code[code_idx++];
        opcode_t opcode = opcode_list[code];

        instruction_t instruction = { .code = code };

        for (int j = 0; j < opcode.param_count; j++) {
            parameter_t parameter = { 0 };
            if (opcode.param_sizes[j] == 2) {
                parameter.type = PARAMETER_TYPE_U16;
                parameter.value.u16 = (event.code[code_idx + 1] << 8) | event.code[code_idx];
                code_idx += 2;
            } else {
                parameter.type = PARAMETER_TYPE_U8;
                parameter.value.u8 = event.code[code_idx];
                code_idx += 1;
            }

            instruction.parameters[j] = parameter;
        }
        instructions[(*count)++] = instruction;
        ASSERT(*count < EVENT_INSTRUCTION_MAX, "Instruction count exceeded");
    }

    return instructions;
}

const opcode_t opcode_list[OPCODE_ID_MAX] = {
    [0x10] = { 0x10, "DisplayMessage", { 1, 1, 2, 1, 1, 1, 2, 2, 2, 1 }, 10 },
    [0x11] = { 0x11, "UnitAnim", { 1, 1, 1, 1, 1 }, 5 },
    [0x12] = { 0x12, "Unknown(0x12)", { 2 }, 1 }, // Chatper 3 Initialization
    [0x13] = { 0x13, "ChangeMapBeta", { 1, 1 }, 2 },
    [0x16] = { 0x16, "Pause", { 0 }, 1 },
    [0x18] = { 0x18, "Effect", { 2, 1, 1, 1, 1 }, 5 },
    [0x19] = { 0x19, "Camera", { 2, 2, 2, 2, 2, 2, 2, 2 }, 8 },
    [0x1A] = { 0x1A, "MapDarkness", { 1, 1, 1, 1, 1 }, 5 },
    [0x1B] = { 0x1B, "MapLight", { 2, 2, 2, 2, 2, 2, 2 }, 7 },
    [0x1C] = { 0x1C, "EventSpeed", { 1 }, 1 },
    [0x1D] = { 0x1D, "CameraFusionStart", { 0 }, 0 },
    [0x1E] = { 0x1E, "CameraFusionEnd", { 0 }, 0 },
    [0x1F] = { 0x1F, "Focus", { 1, 1, 1, 1, 1 }, 5 },
    [0x21] = { 0x21, "SoundEffect", { 2 }, 1 },
    [0x22] = { 0x22, "SwitchTrack", { 1, 1, 1 }, 3 },
    [0x27] = { 0x27, "ReloadMapState", { 0 }, 0 },
    [0x28] = { 0x28, "WalkTo", { 1, 1, 1, 1, 1, 1, 1, 1 }, 8 },
    [0x29] = { 0x29, "WaitWalk", { 1, 1 }, 2 },
    [0x2A] = { 0x2A, "BlockStart", { 0 }, 0 },
    [0x2B] = { 0x2B, "BlockEnd", { 0 }, 0 },
    [0x2C] = { 0x2C, "FaceUnit2", { 1, 1, 1, 1, 1, 1, 1 }, 7 },
    [0x2D] = { 0x2D, "RotateUnit", { 1, 1, 1, 1, 1, 1 }, 6 },
    [0x2E] = { 0x2E, "Background", { 1, 1, 1, 1, 1, 1, 1, 1 }, 8 },
    [0x31] = { 0x31, "ColorBGBeta", { 1, 1, 1, 1, 1 }, 5 },
    [0x32] = { 0x32, "ColorUnit", { 1, 1, 1, 1, 1, 1, 1 }, 7 },
    [0x33] = { 0x33, "ColorField", { 1, 1, 1, 1, 1 }, 5 },
    [0x38] = { 0x38, "FocusSpeed", { 2 }, 1 },
    [0x39] = { 0x39, "Unknown(0x39)", { 0 }, 0 },
    [0x3A] = { 0x3A, "Unknown(0x3A)", { 0 }, 0 },
    [0x3B] = { 0x3B, "SpriteMove", { 1, 1, 2, 2, 2, 1, 1, 2 }, 8 },
    [0x3C] = { 0x3C, "Weather", { 1, 1 }, 2 },
    [0x3D] = { 0x3D, "RemoveUnit", { 1, 1 }, 2 },
    [0x3E] = { 0x3E, "ColorScreen", { 1, 1, 1, 1, 1, 1, 1, 2 }, 8 },
    [0x40] = { 0x40, "Unknown(0x40)", { 0 }, 0 },
    [0x41] = { 0x41, "EarthquakeStart", { 1, 1, 1, 1 }, 4 },
    [0x42] = { 0x42, "EarthquakeEnd", { 0 }, 0 },
    [0x43] = { 0x43, "CallFunction", { 1 }, 1 },
    [0x44] = { 0x44, "Draw", { 1, 1 }, 2 },
    [0x45] = { 0x45, "AddUnit", { 1, 1, 1 }, 3 },
    [0x46] = { 0x46, "Erase", { 1, 1 }, 2 },
    [0x47] = { 0x47, "AddGhostUnit", { 1, 1, 1, 1, 1, 1, 1, 1 }, 8 },
    [0x48] = { 0x48, "WaitAddUnit", { 0 }, 0 },
    [0x49] = { 0x49, "AddUnitStart", { 0 }, 0 },
    [0x4A] = { 0x4A, "AddUnitEnd", { 0 }, 0 },
    [0x4B] = { 0x4B, "WaitAddUnitEnd", { 0 }, 0 },
    [0x4C] = { 0x4C, "ChangeMap", { 1, 1 }, 2 },
    [0x4D] = { 0x4D, "Reveal", { 1 }, 1 },
    [0x4E] = { 0x4E, "UnitShadow", { 1, 1, 1 }, 3 },
    [0x50] = { 0x50, "PortraitCol", { 1 }, 1 },
    [0x51] = { 0x51, "ChangeDialog", { 1, 2, 1, 1 }, 4 },
    [0x53] = { 0x53, "FaceUnit", { 1, 1, 1, 1, 1, 1, 1 }, 7 },
    [0x54] = { 0x54, "Use3DObject", { 1, 1 }, 2 },
    [0x55] = { 0x55, "UseFieldObject", { 1, 1 }, 2 },
    [0x56] = { 0x56, "Wait3DObject", { 0 }, 0 },
    [0x57] = { 0x57, "WaitFieldObject", { 0 }, 0 },
    [0x58] = { 0x58, "LoadEVTCHR", { 1, 1, 1 }, 3 },
    [0x59] = { 0x59, "SaveEVTCHR", { 1 }, 1 },
    [0x5A] = { 0x5A, "SaveEVTCHRClear", { 1 }, 1 },
    [0x5B] = { 0x5B, "LoadEVTCHRClear", { 1 }, 1 },
    [0x5F] = { 0x5F, "WarpUnit", { 1, 1, 1, 1, 1, 1 }, 6 },
    [0x60] = { 0x60, "FadeSound", { 1, 1 }, 2 },
    [0x63] = { 0x63, "CameraSpeedCurve", { 1 }, 1 },
    [0x64] = { 0x64, "WaitRotateUnit", { 1, 1 }, 2 },
    [0x65] = { 0x65, "WaitRotateAll", { 0 }, 0 },
    [0x66] = { 0x66, "Unknown(0x66)", { 0 }, 0 },
    [0x68] = { 0x68, "MirrorSprite", { 1, 1, 1 }, 3 },
    [0x69] = { 0x69, "FaceTile", { 1, 1, 1, 1, 1, 1, 1, 1 }, 8 },
    [0x6A] = { 0x6A, "EditBGSound", { 1, 1, 1, 1, 1 }, 5 },
    [0x6B] = { 0x6B, "BGSound", { 1, 1, 1, 1, 1 }, 5 },
    [0x6D] = { 0x6D, "Unknown(0x6D)", { 1, 1 }, 2 },
    [0x6E] = { 0x6E, "SpriteMoveBeta", { 1, 1, 2, 2, 2, 1, 1, 2 }, 8 },
    [0x6F] = { 0x6F, "WaitSpriteMove", { 1, 1 }, 2 },
    [0x70] = { 0x70, "Jump", { 1, 1, 1, 1 }, 4 },
    [0x71] = { 0x71, "Unknown(0x71)", { 1, 1 }, 2 },
    [0x73] = { 0x73, "Unknown(0x73)", { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 }, 14 },
    [0x75] = { 0x75, "Unknown(0x75)", { 0 }, 0 },
    [0x76] = { 0x76, "DarkScreen", { 1, 1, 1, 1, 1, 1 }, 6 },
    [0x77] = { 0x77, "RemoveDarkScreen", { 0 }, 0 },
    [0x78] = { 0x78, "DisplayConditions", { 1, 1 }, 2 },
    [0x79] = { 0x79, "WalkToAnim", { 1, 1, 2 }, 3 },
    [0x7A] = { 0x7A, "DismissUnit", { 1, 1 }, 2 },
    [0x7B] = { 0x7B, "Unknown(0x7B)", { 1, 1 }, 2 },
    [0x7C] = { 0x7C, "Unknown(0x7C)", { 0 }, 0 },
    [0x7D] = { 0x7D, "ShowGraphic", { 1 }, 1 },
    [0x7E] = { 0x7E, "WaitValue", { 2, 2 }, 2 },
    [0x7F] = { 0x7F, "EVTCHRPalette", { 1, 1, 1, 1 }, 4 },
    [0x80] = { 0x80, "March", { 1, 1, 1 }, 3 },
    [0x82] = { 0x82, "Unknown(0x82)", { 0 }, 0 },
    [0x83] = { 0x83, "ChangeStats", { 1, 1, 1, 2 }, 4 },
    [0x84] = { 0x84, "PlayTune", { 1 }, 1 },
    [0x85] = { 0x85, "UnlockDate", { 1 }, 1 },
    [0x86] = { 0x86, "TempWeapon", { 1, 1, 1 }, 3 },
    [0x87] = { 0x87, "Arrow", { 1, 1, 1, 1 }, 4 },
    [0x88] = { 0x88, "MapUnfreeze", { 0 }, 0 },
    [0x89] = { 0x89, "MapFreeze", { 0 }, 0 },
    [0x8A] = { 0x8A, "EffectStart", { 0 }, 0 },
    [0x8B] = { 0x8B, "EffectEnd", { 0 }, 0 },
    [0x8C] = { 0x8C, "UnitAnimRotate", { 1, 1, 1, 1, 1, 1 }, 6 },
    [0x8E] = { 0x8E, "WaitGraphicPrint", { 0 }, 0 },
    [0x8F] = { 0x8F, "Unknown(0x8F)", { 0 }, 0 },
    [0x90] = { 0x90, "Unknown(0x90)", { 0 }, 0 },
    [0x91] = { 0x91, "ShowMapTitle", { 1, 1, 1 }, 3 },
    [0x92] = { 0x92, "InflictStatus", { 1, 1, 1, 1, 1 }, 5 },
    [0x93] = { 0x93, "Unknown(0x93)", { 0 }, 0 },
    [0x94] = { 0x94, "TeleportOut", { 1, 1 }, 2 },
    [0x96] = { 0x96, "AppendMapState", { 0 }, 0 },
    [0x97] = { 0x97, "ResetPalette", { 1, 1 }, 2 },
    [0x98] = { 0x98, "TeleportIn", { 1, 1 }, 2 },
    [0x99] = { 0x99, "BlueRemoveUnit", { 1, 1 }, 2 },
    [0xA0] = { 0xA0, "LTE", { 0 }, 0 },
    [0xA1] = { 0xA1, "GTE", { 0 }, 0 },
    [0xA2] = { 0xA2, "EQ", { 0 }, 0 },
    [0xA3] = { 0xA3, "NEQ", { 0 }, 0 },
    [0xA4] = { 0xA4, "LT", { 0 }, 0 },
    [0xA5] = { 0xA5, "GT", { 0 }, 0 },
    [0xB0] = { 0xB0, "ADD", { 2, 2 }, 2 },
    [0xB1] = { 0xB1, "ADDVar", { 2, 2 }, 2 },
    [0xB2] = { 0xB2, "SUB", { 2, 2 }, 2 },
    [0xB3] = { 0xB3, "SUBVar", { 2, 2 }, 2 },
    [0xB4] = { 0xB4, "MULT", { 2, 2 }, 2 },
    [0xB5] = { 0xB5, "MULTVar", { 2, 2 }, 2 },
    [0xB6] = { 0xB6, "DIV", { 2, 2 }, 2 },
    [0xB7] = { 0xB7, "DIVVar", { 2, 2 }, 2 },
    [0xB8] = { 0xB8, "MOD", { 2, 2 }, 2 },
    [0xB9] = { 0xB9, "MODVar", { 2, 2 }, 2 },
    [0xBA] = { 0xBA, "AND", { 2, 2 }, 2 },
    [0xBB] = { 0xBB, "ANDVar", { 2, 2 }, 2 },
    [0xBC] = { 0xBC, "OR", { 2, 2 }, 2 },
    [0xBD] = { 0xBD, "ORVar", { 2, 2 }, 2 },
    [0xBE] = { 0xBE, "ZERO", { 2 }, 1 },
    [0xD0] = { 0xD0, "JumpForwardIfZero", { 1 }, 1 },
    [0xD1] = { 0xD1, "JumpForward", { 1 }, 1 },
    [0xD2] = { 0xD2, "ForwardTarget", { 1 }, 1 },
    [0xD3] = { 0xD3, "JumpBack", { 1 }, 1 },
    [0xD4] = { 0xD4, "Unknown(0xD4)", { 0 }, 0 },
    [0xD5] = { 0xD5, "BackTarget", { 1 }, 1 },
    [0xDB] = { 0xDB, "EventEnd", { 0 }, 0 },
    [0xE3] = { 0xE3, "EventEnd2", { 0 }, 0 },
    [0xE5] = { 0xE5, "WaitForInstruction", { 1, 1 }, 2 },
    [0xF0] = { 0xF0, "Unknown(0xF0)", { 0 }, 0 },
    [0xF1] = { 0xF1, "Wait", { 2 }, 1 },
    [0xF2] = { 0xF2, "Pad", { 0 }, 0 },
};
