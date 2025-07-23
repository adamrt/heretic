#pragma once

#include "defines.h"

constexpr int OPCODE_PARAM_MAX = 14;

// clang-format off
typedef enum {
    OPCODE_ID_DISPLAYMESSAGE     = 0x10, // 16
    OPCODE_ID_UNITANIM           = 0x11, // 17
    OPCODE_ID_UNKNOWN_0x12       = 0x12, // 18
    OPCODE_ID_CHANGEMAPBETA      = 0x13, // 19
    OPCODE_ID_PAUSE              = 0x16, // 22
    OPCODE_ID_EFFECT             = 0x18, // 24
    OPCODE_ID_CAMERA             = 0x19, // 25
    OPCODE_ID_MAPDARKNESS        = 0x1A, // 26
    OPCODE_ID_MAPLIGHT           = 0x1B, // 27
    OPCODE_ID_EVENTSPEED         = 0x1C, // 28
    OPCODE_ID_CAMERAFUSIONSTART  = 0x1D, // 29
    OPCODE_ID_CAMERAFUSIONEND    = 0x1E, // 30
    OPCODE_ID_FOCUS              = 0x1F, // 31
    OPCODE_ID_SOUNDEFFECT        = 0x21, // 33
    OPCODE_ID_SWITCHTRACK        = 0x22, // 34
    OPCODE_ID_RELOADMAPSTATE     = 0x27, // 39
    OPCODE_ID_WALKTO             = 0x28, // 40
    OPCODE_ID_WAITWALK           = 0x29, // 41
    OPCODE_ID_BLOCKSTART         = 0x2A, // 42
    OPCODE_ID_BLOCKEND           = 0x2B, // 43
    OPCODE_ID_FACEUNIT2          = 0x2C, // 44
    OPCODE_ID_ROTATEUNIT         = 0x2D, // 45
    OPCODE_ID_BACKGROUND         = 0x2E, // 46
    OPCODE_ID_COLORBGBETA        = 0x31, // 49
    OPCODE_ID_COLORUNIT          = 0x32, // 50
    OPCODE_ID_COLORFIELD         = 0x33, // 51
    OPCODE_ID_FOCUSSPEED         = 0x38, // 56
    OPCODE_ID_UNKNOWN_0x39       = 0x39, // 57
    OPCODE_ID_UNKNOWN_0x3A       = 0x3A, // 58
    OPCODE_ID_SPRITEMOVE         = 0x3B, // 59
    OPCODE_ID_WEATHER            = 0x3C, // 60
    OPCODE_ID_REMOVEUNIT         = 0x3D, // 61
    OPCODE_ID_COLORSCREEN        = 0x3E, // 62
    OPCODE_ID_UNKNOWN_0x40       = 0x40, // 64
    OPCODE_ID_EARTHQUAKESTART    = 0x41, // 65
    OPCODE_ID_EARTHQUAKEEND      = 0x42, // 66
    OPCODE_ID_CALLFUNCTION       = 0x43, // 67
    OPCODE_ID_DRAW               = 0x44, // 68
    OPCODE_ID_ADDUNIT            = 0x45, // 69
    OPCODE_ID_ERASE              = 0x46, // 70
    OPCODE_ID_ADDGHOSTUNIT       = 0x47, // 71
    OPCODE_ID_WAITADDUNIT        = 0x48, // 72
    OPCODE_ID_ADDUNITSTART       = 0x49, // 73
    OPCODE_ID_ADDUNITEND         = 0x4A, // 74
    OPCODE_ID_WAITADDUNITEND     = 0x4B, // 75
    OPCODE_ID_CHANGEMAP          = 0x4C, // 76
    OPCODE_ID_REVEAL             = 0x4D, // 77
    OPCODE_ID_UNITSHADOW         = 0x4E, // 78
    OPCODE_ID_PORTRAITCOL        = 0x50, // 80
    OPCODE_ID_CHANGEDIALOG       = 0x51, // 81
    OPCODE_ID_FACEUNIT           = 0x53, // 83
    OPCODE_ID_USE3DOBJECT        = 0x54, // 84
    OPCODE_ID_USEFIELDOBJECT     = 0x55, // 85
    OPCODE_ID_WAIT3DOBJECT       = 0x56, // 86
    OPCODE_ID_WAITFIELDOBJECT    = 0x57, // 87
    OPCODE_ID_LOADEVTCHR         = 0x58, // 88
    OPCODE_ID_SAVEEVTCHR         = 0x59, // 89
    OPCODE_ID_SAVEEVTCHRCLEAR    = 0x5A, // 90
    OPCODE_ID_LOADEVTCHRCLEAR    = 0x5B, // 91
    OPCODE_ID_WARPUNIT           = 0x5F, // 95
    OPCODE_ID_FADESOUND          = 0x60, // 96
    OPCODE_ID_CAMERASPEEDCURVE   = 0x63, // 99
    OPCODE_ID_WAITROTATEUNIT     = 0x64, // 100
    OPCODE_ID_WAITROTATEALL      = 0x65, // 101
    OPCODE_ID_UNKNOWN_0x66       = 0x66, // 102
    OPCODE_ID_MIRRORSPRITE       = 0x68, // 104
    OPCODE_ID_FACETILE           = 0x69, // 105
    OPCODE_ID_EDITBGSOUND        = 0x6A, // 106
    OPCODE_ID_BGSOUND            = 0x6B, // 107
    OPCODE_ID_UNKNOWN_0x6D       = 0x6D, // 109
    OPCODE_ID_SPRITEMOVEBETA     = 0x6E, // 110
    OPCODE_ID_WAITSPRITEMOVE     = 0x6F, // 111
    OPCODE_ID_JUMP               = 0x70, // 112
    OPCODE_ID_UNKNOWN_0x71       = 0x71, // 113
    OPCODE_ID_UNKNOWN_0x73       = 0x73, // 115
    OPCODE_ID_UNKNOWN_0x75       = 0x75, // 117
    OPCODE_ID_DARKSCREEN         = 0x76, // 118
    OPCODE_ID_REMOVEDARKSCREEN   = 0x77, // 119
    OPCODE_ID_DISPLAYCONDITIONS  = 0x78, // 120
    OPCODE_ID_WALKTOANIM         = 0x79, // 121
    OPCODE_ID_DISMISSUNIT        = 0x7A, // 122
    OPCODE_ID_UNKNOWN_0x7B       = 0x7B, // 123
    OPCODE_ID_UNKNOWN_0x7C       = 0x7C, // 124
    OPCODE_ID_SHOWGRAPHIC        = 0x7D, // 125
    OPCODE_ID_WAITVALUE          = 0x7E, // 126
    OPCODE_ID_EVTCHRPALETTE      = 0x7F, // 127
    OPCODE_ID_MARCH              = 0x80, // 128
    OPCODE_ID_UNKNOWN_0x82       = 0x82, // 130
    OPCODE_ID_CHANGESTATS        = 0x83, // 131
    OPCODE_ID_PLAYTUNE           = 0x84, // 132
    OPCODE_ID_UNLOCKDATE         = 0x85, // 133
    OPCODE_ID_TEMPWEAPON         = 0x86, // 134
    OPCODE_ID_ARROW              = 0x87, // 135
    OPCODE_ID_MAPUNFREEZE        = 0x88, // 136
    OPCODE_ID_MAPFREEZE          = 0x89, // 137
    OPCODE_ID_EFFECTSTART        = 0x8A, // 138
    OPCODE_ID_EFFECTEND          = 0x8B, // 139
    OPCODE_ID_UNITANIMROTATE     = 0x8C, // 140
    OPCODE_ID_WAITGRAPHICPRINT   = 0x8E, // 142
    OPCODE_ID_UNKNOWN_0x8F       = 0x8F, // 143
    OPCODE_ID_UNKNOWN_0x90       = 0x90, // 144
    OPCODE_ID_SHOWMAPTITLE       = 0x91, // 145
    OPCODE_ID_INFLICTSTATUS      = 0x92, // 146
    OPCODE_ID_UNKNOWN_0x93       = 0x93, // 147
    OPCODE_ID_TELEPORTOUT        = 0x94, // 148
    OPCODE_ID_APPENDMAPSTATE     = 0x96, // 150
    OPCODE_ID_RESETPALETTE       = 0x97, // 151
    OPCODE_ID_TELEPORTIN         = 0x98, // 152
    OPCODE_ID_BLUEREMOVEUNIT     = 0x99, // 153
    OPCODE_ID_LTE                = 0xA0, // 160
    OPCODE_ID_GTE                = 0xA1, // 161
    OPCODE_ID_EQ                 = 0xA2, // 162
    OPCODE_ID_NEQ                = 0xA3, // 163
    OPCODE_ID_LT                 = 0xA4, // 164
    OPCODE_ID_GT                 = 0xA5, // 165
    OPCODE_ID_ADD                = 0xB0, // 176
    OPCODE_ID_ADDVAR             = 0xB1, // 177
    OPCODE_ID_SUB                = 0xB2, // 178
    OPCODE_ID_SUBVAR             = 0xB3, // 179
    OPCODE_ID_MULT               = 0xB4, // 180
    OPCODE_ID_MULTVAR            = 0xB5, // 181
    OPCODE_ID_DIV                = 0xB6, // 182
    OPCODE_ID_DIVVAR             = 0xB7, // 183
    OPCODE_ID_MOD                = 0xB8, // 184
    OPCODE_ID_MODVAR             = 0xB9, // 185
    OPCODE_ID_AND                = 0xBA, // 186
    OPCODE_ID_ANDVAR             = 0xBB, // 187
    OPCODE_ID_OR                 = 0xBC, // 188
    OPCODE_ID_ORVAR              = 0xBD, // 189
    OPCODE_ID_ZERO               = 0xBE, // 190
    OPCODE_ID_JUMPFORWARDIFZERO  = 0xD0, // 208
    OPCODE_ID_JUMPFORWARD        = 0xD1, // 209
    OPCODE_ID_FORWARDTARGET      = 0xD2, // 210
    OPCODE_ID_JUMPBACK           = 0xD3, // 211
    OPCODE_ID_UNKNOWN_0xD4       = 0xD4, // 212
    OPCODE_ID_BACKTARGET         = 0xD5, // 213
    OPCODE_ID_EVENTEND           = 0xDB, // 219
    OPCODE_ID_EVENTEND2          = 0xE3, // 227
    OPCODE_ID_WAITFORINSTRUCTION = 0xE5, // 229
    OPCODE_ID_UNKNOWN_0xF0       = 0xF0, // 240
    OPCODE_ID_WAIT               = 0xF1, // 241
    OPCODE_ID_PAD                = 0xF2, // 242
    OPCODE_ID_MAX                = 0xF3, // 243
} opcode_id_t;
// clang-format on

// opcode_desc_t is a struct that describes an opcode and its parameters.
typedef struct {
    opcode_id_t id;
    const char* name;
    u8 param_sizes[OPCODE_PARAM_MAX];
    u8 param_count;
} opcode_desc_t;

extern const opcode_desc_t opcode_desc_list[];
