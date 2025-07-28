#pragma once

#include "defines.h"

enum {
    OPCODE_PARAM_MAX = 14,
};

#define PARAMS(...) { __VA_ARGS__ }

#define OPCODE_LIST                                                                                     \
    X(OPCODE_DISPLAYMESSAGE, 0x10, "DisplayMessage", PARAMS(1, 1, 2, 1, 1, 1, 2, 2, 2, 1), 10)          \
    X(OPCODE_UNITANIM, 0x11, "UnitAnim", PARAMS(1, 1, 1, 1, 1), 5)                                      \
    X(OPCODE_UNKNOWN_0x12, 0x12, "Unknown(0x12)", PARAMS(2), 1)                                         \
    X(OPCODE_CHANGEMAPBETA, 0x13, "ChangeMapBeta", PARAMS(1, 1), 2)                                     \
    X(OPCODE_PAUSE, 0x16, "Pause", PARAMS(0), 0)                                                        \
    X(OPCODE_EFFECT, 0x18, "Effect", PARAMS(2, 1, 1, 1, 1), 5)                                          \
    X(OPCODE_CAMERA, 0x19, "Camera", PARAMS(2, 2, 2, 2, 2, 2, 2, 2), 8)                                 \
    X(OPCODE_MAPDARKNESS, 0x1A, "MapDarkness", PARAMS(1, 1, 1, 1, 1), 5)                                \
    X(OPCODE_MAPLIGHT, 0x1B, "MapLight", PARAMS(2, 2, 2, 2, 2, 2, 2), 7)                                \
    X(OPCODE_EVENTSPEED, 0x1C, "EventSpeed", PARAMS(1), 1)                                              \
    X(OPCODE_CAMERAFUSIONSTART, 0x1D, "CameraFusionStart", PARAMS(0), 0)                                \
    X(OPCODE_CAMERAFUSIONEND, 0x1E, "CameraFusionEnd", PARAMS(0), 0)                                    \
    X(OPCODE_FOCUS, 0x1F, "Focus", PARAMS(1, 1, 1, 1, 1), 5)                                            \
    X(OPCODE_SOUNDEFFECT, 0x21, "SoundEffect", PARAMS(2), 1)                                            \
    X(OPCODE_SWITCHTRACK, 0x22, "SwitchTrack", PARAMS(1, 1, 1), 3)                                      \
    X(OPCODE_RELOADMAPSTATE, 0x27, "ReloadMapState", PARAMS(0), 0)                                      \
    X(OPCODE_WALKTO, 0x28, "WalkTo", PARAMS(1, 1, 1, 1, 1, 1, 1, 1), 8)                                 \
    X(OPCODE_WAITWALK, 0x29, "WaitWalk", PARAMS(1, 1), 2)                                               \
    X(OPCODE_BLOCKSTART, 0x2A, "BlockStart", PARAMS(0), 0)                                              \
    X(OPCODE_BLOCKEND, 0x2B, "BlockEnd", PARAMS(0), 0)                                                  \
    X(OPCODE_FACEUNIT2, 0x2C, "FaceUnit2", PARAMS(1, 1, 1, 1, 1, 1, 1), 7)                              \
    X(OPCODE_ROTATEUNIT, 0x2D, "RotateUnit", PARAMS(1, 1, 1, 1, 1, 1), 6)                               \
    X(OPCODE_BACKGROUND, 0x2E, "Background", PARAMS(1, 1, 1, 1, 1, 1, 1, 1), 8)                         \
    X(OPCODE_COLORBGBETA, 0x31, "ColorBGBeta", PARAMS(1, 1, 1, 1, 1), 5)                                \
    X(OPCODE_COLORUNIT, 0x32, "ColorUnit", PARAMS(1, 1, 1, 1, 1, 1, 1), 7)                              \
    X(OPCODE_COLORFIELD, 0x33, "ColorField", PARAMS(1, 1, 1, 1, 1), 5)                                  \
    X(OPCODE_FOCUSSPEED, 0x38, "FocusSpeed", PARAMS(2), 1)                                              \
    X(OPCODE_UNKNOWN_0x39, 0x39, "Unknown(0x39)", PARAMS(0), 0)                                         \
    X(OPCODE_UNKNOWN_0x3A, 0x3A, "Unknown(0x3A)", PARAMS(0), 0)                                         \
    X(OPCODE_SPRITEMOVE, 0x3B, "SpriteMove", PARAMS(1, 1, 2, 2, 2, 1, 1, 2), 8)                         \
    X(OPCODE_WEATHER, 0x3C, "Weather", PARAMS(1, 1), 2)                                                 \
    X(OPCODE_REMOVEUNIT, 0x3D, "RemoveUnit", PARAMS(1, 1), 2)                                           \
    X(OPCODE_COLORSCREEN, 0x3E, "ColorScreen", PARAMS(1, 1, 1, 1, 1, 1, 1, 2), 8)                       \
    X(OPCODE_UNKNOWN_0x40, 0x40, "Unknown(0x40)", PARAMS(0), 0)                                         \
    X(OPCODE_EARTHQUAKESTART, 0x41, "EarthquakeStart", PARAMS(1, 1, 1, 1), 4)                           \
    X(OPCODE_EARTHQUAKEEND, 0x42, "EarthquakeEnd", PARAMS(0), 0)                                        \
    X(OPCODE_CALLFUNCTION, 0x43, "CallFunction", PARAMS(1), 1)                                          \
    X(OPCODE_DRAW, 0x44, "Draw", PARAMS(1, 1), 2)                                                       \
    X(OPCODE_ADDUNIT, 0x45, "AddUnit", PARAMS(1, 1, 1), 3)                                              \
    X(OPCODE_ERASE, 0x46, "Erase", PARAMS(1, 1), 2)                                                     \
    X(OPCODE_ADDGHOSTUNIT, 0x47, "AddGhostUnit", PARAMS(1, 1, 1, 1, 1, 1, 1, 1), 8)                     \
    X(OPCODE_WAITADDUNIT, 0x48, "WaitAddUnit", PARAMS(0), 0)                                            \
    X(OPCODE_ADDUNITSTART, 0x49, "AddUnitStart", PARAMS(0), 0)                                          \
    X(OPCODE_ADDUNITEND, 0x4A, "AddUnitEnd", PARAMS(0), 0)                                              \
    X(OPCODE_WAITADDUNITEND, 0x4B, "WaitAddUnitEnd", PARAMS(0), 0)                                      \
    X(OPCODE_CHANGEMAP, 0x4C, "ChangeMap", PARAMS(1, 1), 2)                                             \
    X(OPCODE_REVEAL, 0x4D, "Reveal", PARAMS(1), 1)                                                      \
    X(OPCODE_UNITSHADOW, 0x4E, "UnitShadow", PARAMS(1, 1, 1), 3)                                        \
    X(OPCODE_PORTRAITCOL, 0x50, "PortraitCol", PARAMS(1), 1)                                            \
    X(OPCODE_CHANGEDIALOG, 0x51, "ChangeDialog", PARAMS(1, 2, 1, 1), 4)                                 \
    X(OPCODE_FACEUNIT, 0x53, "FaceUnit", PARAMS(1, 1, 1, 1, 1, 1, 1), 7)                                \
    X(OPCODE_USE3DOBJECT, 0x54, "Use3DObject", PARAMS(1, 1), 2)                                         \
    X(OPCODE_USEFIELDOBJECT, 0x55, "UseFieldObject", PARAMS(1, 1), 2)                                   \
    X(OPCODE_WAIT3DOBJECT, 0x56, "Wait3DObject", PARAMS(0), 0)                                          \
    X(OPCODE_WAITFIELDOBJECT, 0x57, "WaitFieldObject", PARAMS(0), 0)                                    \
    X(OPCODE_LOADEVTCHR, 0x58, "LoadEVTCHR", PARAMS(1, 1, 1), 3)                                        \
    X(OPCODE_SAVEEVTCHR, 0x59, "SaveEVTCHR", PARAMS(1), 1)                                              \
    X(OPCODE_SAVEEVTCHRCLEAR, 0x5A, "SaveEVTCHRClear", PARAMS(1), 1)                                    \
    X(OPCODE_LOADEVTCHRCLEAR, 0x5B, "LoadEVTCHRClear", PARAMS(1), 1)                                    \
    X(OPCODE_WARPUNIT, 0x5F, "WarpUnit", PARAMS(1, 1, 1, 1, 1, 1), 6)                                   \
    X(OPCODE_FADESOUND, 0x60, "FadeSound", PARAMS(1, 1), 2)                                             \
    X(OPCODE_CAMERASPEEDCURVE, 0x63, "CameraSpeedCurve", PARAMS(1), 1)                                  \
    X(OPCODE_WAITROTATEUNIT, 0x64, "WaitRotateUnit", PARAMS(1, 1), 2)                                   \
    X(OPCODE_WAITROTATEALL, 0x65, "WaitRotateAll", PARAMS(0), 0)                                        \
    X(OPCODE_UNKNOWN_0x66, 0x66, "Unknown(0x66)", PARAMS(0), 0)                                         \
    X(OPCODE_MIRRORSPRITE, 0x68, "MirrorSprite", PARAMS(1, 1, 1), 3)                                    \
    X(OPCODE_FACETILE, 0x69, "FaceTile", PARAMS(1, 1, 1, 1, 1, 1, 1, 1), 8)                             \
    X(OPCODE_EDITBGSOUND, 0x6A, "EditBGSound", PARAMS(1, 1, 1, 1, 1), 5)                                \
    X(OPCODE_BGSOUND, 0x6B, "BGSound", PARAMS(1, 1, 1, 1, 1), 5)                                        \
    X(OPCODE_UNKNOWN_0x6D, 0x6D, "Unknown(0x6D)", PARAMS(1, 1), 2)                                      \
    X(OPCODE_SPRITEMOVEBETA, 0x6E, "SpriteMoveBeta", PARAMS(1, 1, 2, 2, 2, 1, 1, 2), 8)                 \
    X(OPCODE_WAITSPRITEMOVE, 0x6F, "WaitSpriteMove", PARAMS(1, 1), 2)                                   \
    X(OPCODE_JUMP, 0x70, "Jump", PARAMS(1, 1, 1, 1), 4)                                                 \
    X(OPCODE_UNKNOWN_0x71, 0x71, "Unknown(0x71)", PARAMS(1, 1), 2)                                      \
    X(OPCODE_UNKNOWN_0x73, 0x73, "Unknown(0x73)", PARAMS(1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1), 14) \
    X(OPCODE_UNKNOWN_0x75, 0x75, "Unknown(0x75)", PARAMS(0), 0)                                         \
    X(OPCODE_DARKSCREEN, 0x76, "DarkScreen", PARAMS(1, 1, 1, 1, 1, 1), 6)                               \
    X(OPCODE_REMOVEDARKSCREEN, 0x77, "RemoveDarkScreen", PARAMS(0), 0)                                  \
    X(OPCODE_DISPLAYCONDITIONS, 0x78, "DisplayConditions", PARAMS(1, 1), 2)                             \
    X(OPCODE_WALKTOANIM, 0x79, "WalkToAnim", PARAMS(1, 1, 2), 3)                                        \
    X(OPCODE_DISMISSUNIT, 0x7A, "DismissUnit", PARAMS(1, 1), 2)                                         \
    X(OPCODE_UNKNOWN_0x7B, 0x7B, "Unknown(0x7B)", PARAMS(1, 1), 2)                                      \
    X(OPCODE_UNKNOWN_0x7C, 0x7C, "Unknown(0x7C)", PARAMS(0), 0)                                         \
    X(OPCODE_SHOWGRAPHIC, 0x7D, "ShowGraphic", PARAMS(1), 1)                                            \
    X(OPCODE_WAITVALUE, 0x7E, "WaitValue", PARAMS(2, 2), 2)                                             \
    X(OPCODE_EVTCHRPALETTE, 0x7F, "EVTCHRPalette", PARAMS(1, 1, 1, 1), 4)                               \
    X(OPCODE_MARCH, 0x80, "March", PARAMS(1, 1, 1), 3)                                                  \
    X(OPCODE_UNKNOWN_0x82, 0x82, "Unknown(0x82)", PARAMS(0), 0)                                         \
    X(OPCODE_CHANGESTATS, 0x83, "ChangeStats", PARAMS(1, 1, 1, 2), 4)                                   \
    X(OPCODE_PLAYTUNE, 0x84, "PlayTune", PARAMS(1), 1)                                                  \
    X(OPCODE_UNLOCKDATE, 0x85, "UnlockDate", PARAMS(1), 1)                                              \
    X(OPCODE_TEMPWEAPON, 0x86, "TempWeapon", PARAMS(1, 1, 1), 3)                                        \
    X(OPCODE_ARROW, 0x87, "Arrow", PARAMS(1, 1, 1, 1), 4)                                               \
    X(OPCODE_MAPUNFREEZE, 0x88, "MapUnfreeze", PARAMS(0), 0)                                            \
    X(OPCODE_MAPFREEZE, 0x89, "MapFreeze", PARAMS(0), 0)                                                \
    X(OPCODE_EFFECTSTART, 0x8A, "EffectStart", PARAMS(0), 0)                                            \
    X(OPCODE_EFFECTEND, 0x8B, "EffectEnd", PARAMS(0), 0)                                                \
    X(OPCODE_UNITANIMROTATE, 0x8C, "UnitAnimRotate", PARAMS(1, 1, 1, 1, 1, 1), 6)                       \
    X(OPCODE_WAITGRAPHICPRINT, 0x8E, "WaitGraphicPrint", PARAMS(0), 0)                                  \
    X(OPCODE_UNKNOWN_0x8F, 0x8F, "Unknown(0x8F)", PARAMS(0), 0)                                         \
    X(OPCODE_UNKNOWN_0x90, 0x90, "Unknown(0x90)", PARAMS(0), 0)                                         \
    X(OPCODE_SHOWMAPTITLE, 0x91, "ShowMapTitle", PARAMS(1, 1, 1), 3)                                    \
    X(OPCODE_INFLICTSTATUS, 0x92, "InflictStatus", PARAMS(1, 1, 1, 1, 1), 5)                            \
    X(OPCODE_UNKNOWN_0x93, 0x93, "Unknown(0x93)", PARAMS(0), 0)                                         \
    X(OPCODE_TELEPORTOUT, 0x94, "TeleportOut", PARAMS(1, 1), 2)                                         \
    X(OPCODE_APPENDMAPSTATE, 0x96, "AppendMapState", PARAMS(0), 0)                                      \
    X(OPCODE_RESETPALETTE, 0x97, "ResetPalette", PARAMS(1, 1), 2)                                       \
    X(OPCODE_TELEPORTIN, 0x98, "TeleportIn", PARAMS(1, 1), 2)                                           \
    X(OPCODE_BLUEREMOVEUNIT, 0x99, "BlueRemoveUnit", PARAMS(1, 1), 2)                                   \
    X(OPCODE_LTE, 0xA0, "LTE", PARAMS(0), 0)                                                            \
    X(OPCODE_GTE, 0xA1, "GTE", PARAMS(0), 0)                                                            \
    X(OPCODE_EQ, 0xA2, "EQ", PARAMS(0), 0)                                                              \
    X(OPCODE_NEQ, 0xA3, "NEQ", PARAMS(0), 0)                                                            \
    X(OPCODE_LT, 0xA4, "LT", PARAMS(0), 0)                                                              \
    X(OPCODE_GT, 0xA5, "GT", PARAMS(0), 0)                                                              \
    X(OPCODE_ADD, 0xB0, "ADD", PARAMS(2, 2), 2)                                                         \
    X(OPCODE_ADDVAR, 0xB1, "ADDVar", PARAMS(2, 2), 2)                                                   \
    X(OPCODE_SUB, 0xB2, "SUB", PARAMS(2, 2), 2)                                                         \
    X(OPCODE_SUBVAR, 0xB3, "SUBVar", PARAMS(2, 2), 2)                                                   \
    X(OPCODE_MULT, 0xB4, "MULT", PARAMS(2, 2), 2)                                                       \
    X(OPCODE_MULTVAR, 0xB5, "MULTVar", PARAMS(2, 2), 2)                                                 \
    X(OPCODE_DIV, 0xB6, "DIV", PARAMS(2, 2), 2)                                                         \
    X(OPCODE_DIVVAR, 0xB7, "DIVVar", PARAMS(2, 2), 2)                                                   \
    X(OPCODE_MOD, 0xB8, "MOD", PARAMS(2, 2), 2)                                                         \
    X(OPCODE_MODVAR, 0xB9, "MODVar", PARAMS(2, 2), 2)                                                   \
    X(OPCODE_AND, 0xBA, "AND", PARAMS(2, 2), 2)                                                         \
    X(OPCODE_ANDVAR, 0xBB, "ANDVar", PARAMS(2, 2), 2)                                                   \
    X(OPCODE_OR, 0xBC, "OR", PARAMS(2, 2), 2)                                                           \
    X(OPCODE_ORVAR, 0xBD, "ORVar", PARAMS(2, 2), 2)                                                     \
    X(OPCODE_ZERO, 0xBE, "ZERO", PARAMS(2), 1)                                                          \
    X(OPCODE_JUMPFORWARDIFZERO, 0xD0, "JumpForwardIfZero", PARAMS(1), 1)                                \
    X(OPCODE_JUMPFORWARD, 0xD1, "JumpForward", PARAMS(1), 1)                                            \
    X(OPCODE_FORWARDTARGET, 0xD2, "ForwardTarget", PARAMS(1), 1)                                        \
    X(OPCODE_JUMPBACK, 0xD3, "JumpBack", PARAMS(1), 1)                                                  \
    X(OPCODE_UNKNOWN_0xD4, 0xD4, "Unknown(0xD4)", PARAMS(0), 0)                                         \
    X(OPCODE_BACKTARGET, 0xD5, "BackTarget", PARAMS(1), 1)                                              \
    X(OPCODE_EVENTEND, 0xDB, "EventEnd", PARAMS(0), 0)                                                  \
    X(OPCODE_EVENTEND2, 0xE3, "EventEnd2", PARAMS(0), 0)                                                \
    X(OPCODE_WAITFORINSTRUCTION, 0xE5, "WaitForInstruction", PARAMS(1, 1), 2)                           \
    X(OPCODE_UNKNOWN_0xF0, 0xF0, "Unknown(0xF0)", PARAMS(0), 0)                                         \
    X(OPCODE_WAIT, 0xF1, "Wait", PARAMS(2), 1)                                                          \
    X(OPCODE_PAD, 0xF2, "Pad", PARAMS(0), 0)

typedef enum {
#define X(id, value, name, params, param_count) id = value,
    OPCODE_LIST
#undef X
        OPCODE_COUNT // Automatically represents the count of opcodes
} opcode_e;

// opcode_desc_t is a struct that describes an opcode and its parameters.
typedef struct {
    opcode_e opcode;
    const char* name;
    u8 param_sizes[OPCODE_PARAM_MAX];
    u8 param_count;
} opcode_desc_t;

extern const opcode_desc_t opcode_desc_list[];
