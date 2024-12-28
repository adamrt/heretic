#include "dialog.h"

dialog_t parse_dialog(u8 byte) {
    const u8 TYPE_MASK = 0xF0;      // 11110000
    const u8 ARROW_MASK = 0x0C;     // 00001100
    const u8 ALIGNMENT_MASK = 0x03; // 00000011

    dialog_type_e type = (dialog_type_e)((byte & TYPE_MASK) >> 4);
    dialog_arrow_e arrow = (dialog_arrow_e)((byte & ARROW_MASK) >> 2);
    dialog_alignment_e alignment = (dialog_alignment_e)(byte & ALIGNMENT_MASK);

    dialog_t dialog = {
        .type = type,
        .arrow = arrow,
        .alignment = alignment,
    };
    return dialog;
}

dialog_opening_t parse_dialog_opening(u8 byte) {
    const u8 SPEED_PLUS_50_MASK = 0x01;  // 00000001 (Bit 0)
    const u8 SPEED_MINUS_50_MASK = 0x02; // 00000010 (Bit 1)
    const u8 DARKEN_MASK = 0x04;         // 00000100 (Bit 2)
    const u8 TOGGLE_ARROW_MASK = 0x10;   // 00010000 (Bit 4)

    dialog_opening_t dialog = {};

    if (byte & SPEED_PLUS_50_MASK) {
        dialog.speed = DIALOG_SPEED_PLUS_50;
    } else if (byte & SPEED_MINUS_50_MASK) {
        dialog.speed = DIALOG_SPEED_MINUS_50;
        dialog.remove_bouncing = true;
    }

    if (byte & DARKEN_MASK) {
        dialog.darken = true;
    }

    if (byte & TOGGLE_ARROW_MASK) {
        dialog.toggle_arrow_right = true;
    }

    return dialog;
}
