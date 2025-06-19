#pragma once

#include "defines.h"

#define UNITS_PER_EVENT (16)

// ENTD contains unit data for events (most of them are battles).
//
// Event IDs reference one of the four ENTD files.
//
// Each ENTD file has 128 events (640 bytes each)
// Each event has 16 charaters (40 bytes each)
//
// You can determine which file by the ENTD ID:
// - ENTD1.ENT (IDs 0x000 to 0x07F)
// - ENTD2.ENT (IDs 0x080 to 0x0FF)
// - ENTD3.ENT (IDs 0x100 to 0x17F)
// - ENTD4.ENT (IDs 0x180 to 0x1FF)

typedef enum : u8 {
    FLAGA_MALE = 0x01,
    FLAGA_FEMALE = 0x02,
    FLAGA_MONSTER = 0x04,
    FLAGA_JOIN_AFTER = 0x08,
    FLAGA_LOAD_FORMATION = 0x10,
    FLAGA_HIDE_STATS = 0x20,
    FLAGA_UNKNOWN = 0x40,
    FLAGA_SAVE_FORMATION = 0x80,
} unit_flags_a_e;

typedef enum : u8 {
    FLAGB_ALWAYS_P = 0x01,
    FLAGB_RANDOMLY_P = 0x02,
    FLAGB_TEAM_C = 0x0C, // 2 bits
    FLAGB_CONTROL = 0x10,
    FLAGB_IMMORTAL = 0x20,
    FLAGB_UNKNOWN_1 = 0x40,
    FLAGB_UNKNOWN_2 = 0x80,
} unit_flags_b_e;

typedef enum : u8 {
    FLAGC_FOCUS_U = 0x01,
    FLAGC_STAY_NEAR_XY = 0x02,
    FLAGC_AGGRESSIVE = 0x04,
    FLAGC_DEFENSIVE = 0x08,
    FLAGC_UNKNOWN_1 = 0x10,
    FLAGC_UNKNOWN_2 = 0x20,
    FLAGC_UNKNOWN_3 = 0x40,
    FLAGC_UNKNOWN_4 = 0x80,
} unit_flags_c_e;

typedef enum : u8 {
    DIRECTION_SOUTH = 0x00,
    DIRECTION_EAST = 0x01,
    DIRECTION_NORTH = 0x02,
    DIRECTION_WEST = 0x03,
} direction_e;

typedef struct {
    u8 sprite_set;          // 0x00
    unit_flags_a_e flags_a; // 0x01
    u8 name;                // 0x02
    u8 level;               // 0x03 0xFE = party level - random
    u16 birthday;           // 0x04
    u8 bravery;             // 0x06
    u8 faith;               // 0x07
    u8 job_unlock;          // 0x08
    u8 job_level;           // 0x09
    u8 job;                 // 0x0A
    u8 secondary_job;       // 0x0B
    u16 reaction;           // 0x0C
    u16 support;            // 0x0E
    u16 movement;           // 0x10
    u8 head;                // 0x12
    u8 body;                // 0x13
    u8 accessory;           // 0x14
    u8 right_hand;          // 0x15
    u8 left_hand;           // 0x16
    u8 palette;             // 0x17
    unit_flags_b_e flags_b; // 0x18
    i8 pos_x;               // 0x19
    i8 pos_y;               // 0x1A
    direction_e direction;  // 0x1B
    u16 experience;         // 0x1C
    u8 unknown_1D;          // 0x1D,
    u16 unknown_1E;         // 0x1E,
    u8 unit_id;             // 0x20
    u16 unknown_21;         // 0x21,
    unit_flags_c_e flags_c; // 0x23,
    u8 target_unit_id;      // 0x24
    u32 unknown_25;         // 0x25,
} unit_t;

typedef struct {
    unit_t units[UNITS_PER_EVENT]; // 16 units per event
    u8 count;
} units_t;

units_t unit_get_units(int);
void unit_flags_a_str(unit_flags_a_e, char[static 64]);
void unit_flags_b_str(unit_flags_b_e, char[static 64]);
void unit_flags_c_str(unit_flags_c_e, char[static 64]);
void unit_bday_str(u16, char[static 64]);
void unit_level_str(u8, char[static 64]);
void unit_brave_faith_str(u8, u8, char[static 64]);

const char* unit_job_str(u8);
const char* unit_name_str(u8);
const char* unit_item_str(u8);
const char* unit_skill_str(u8);
const char* unit_ability_str(u16);
const char* unit_direction_str(direction_e); // This should be moved somewhere more generic
