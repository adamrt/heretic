#pragma once

#include "defines.h"
#include <stdbool.h>

typedef enum {
    DIALOG_TYPE_UNUSED = 0x0,
    DIALOG_TYPE_CENTERED = 0x0,
    DIALOG_TYPE_3LINE_PORTRAIT = 0x1,
    DIALOG_TYPE_CHECK = 0x2,
    DIALOG_TYPE_HELP = 0x3,
    DIALOG_TYPE_2LINE = 0x4,
    DIALOG_TYPE_8LINE = 0x5,
    DIALOG_TYPE_UNNAMED_1_14 = 0x6,
    DIALOG_TYPE_3LINE_PORTRAIT_7 = 0x7,
    // The following are closed later with fn_51_change_dialog
    DIALOG_TYPE_VARIOUS_8 = 0x8,
    DIALOG_TYPE_3LINE_PORTRAIT_9 = 0x9,
    DIALOG_TYPE_CHECK_A = 0xA,
    DIALOG_TYPE_HELP_B = 0xB,
    DIALOG_TYPE_UNNAMED_C = 0xC,
    DIALOG_TYPE_UNNAMED_D = 0xD,
    DIALOG_TYPE_UNNAMED_E = 0xE,
    DIALOG_TYPE_3LINE_PORTRAIT_F = 0xF
} dialog_type_e;

typedef enum {
    DIALOG_ARROW_DEFAULT = 0x0,
    DIALOG_ARROW_THINKING_BUBBLES = 0x1,
    DIALOG_ARROW_REMOVE = 0x2,
    DIALOG_ARROW_RESERVED = 0x3
} dialog_arrow_e;

typedef enum {
    DIALOG_ALIGNMENT_DEFAULT = 0x0,
    DIALOG_ALIGNMENT_TOP = 0x1,
    DIALOG_ALIGNMENT_BOTTOM = 0x2,
    DIALOG_ALIGNMENT_CENTER = 0x3
} dialog_alignment_e;

typedef enum {
    DIALOG_SPEED_DEFAULT = 0,
    DIALOG_SPEED_PLUS_50 = 1,
    DIALOG_SPEED_MINUS_50 = 2
} dialog_speed_e;

typedef struct {
    dialog_speed_e speed;
    bool remove_bouncing;
    bool darken;
    bool toggle_arrow_right;
} dialog_opening_t;

typedef struct {
    dialog_type_e type;
    dialog_arrow_e arrow;
    dialog_alignment_e alignment;
} dialog_t;

dialog_t parse_dialog(u8);
dialog_opening_t parse_dialog_opening(u8);
