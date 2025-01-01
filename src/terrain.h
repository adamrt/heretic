#pragma once

#include "defines.h"
#include "span.h"

#define SURFACE_INDEX                                   \
    X(SURFACE_NATURAL_SURFACE, 0x00, "NATURAL_SURFACE") \
    X(SURFACE_SAND, 0x01, "SAND")                       \
    X(SURFACE_STALACTITE, 0x02, "STALACTITE")           \
    X(SURFACE_GRASSLAND, 0x03, "GRASSLAND")             \
    X(SURFACE_THICKET, 0x04, "THICKET")                 \
    X(SURFACE_SNOW, 0x05, "SNOW")                       \
    X(SURFACE_ROCKY_CLIFF, 0x06, "ROCKY_CLIFF")         \
    X(SURFACE_GRAVEL, 0x07, "GRAVEL")                   \
    X(SURFACE_WASTELAND, 0x08, "WASTELAND")             \
    X(SURFACE_SWAMP, 0x09, "SWAMP")                     \
    X(SURFACE_MARSH, 0x0A, "MARSH")                     \
    X(SURFACE_POISONED_MARSH, 0x0B, "POISONED_MARSH")   \
    X(SURFACE_LAVA_ROCKS, 0x0C, "LAVA_ROCKS")           \
    X(SURFACE_ICE, 0x0D, "ICE")                         \
    X(SURFACE_WATERWAY, 0x0E, "WATERWAY")               \
    X(SURFACE_RIVER, 0x0F, "RIVER")                     \
    X(SURFACE_LAKE, 0x10, "LAKE")                       \
    X(SURFACE_SEA, 0x11, "SEA")                         \
    X(SURFACE_LAVA, 0x12, "LAVA")                       \
    X(SURFACE_ROAD, 0x13, "ROAD")                       \
    X(SURFACE_WOODEN_FLOOR, 0x14, "WOODEN_FLOOR")       \
    X(SURFACE_STONE_FLOOR, 0x15, "STONE_FLOOR")         \
    X(SURFACE_ROOF, 0x16, "ROOF")                       \
    X(SURFACE_STONEWALL, 0x17, "STONEWALL")             \
    X(SURFACE_SKY, 0x18, "SKY")                         \
    X(SURFACE_DARKNESS, 0x19, "DARKNESS")               \
    X(SURFACE_SALT, 0x1A, "SALT")                       \
    X(SURFACE_BOOK, 0x1B, "BOOK")                       \
    X(SURFACE_OBSTACLE, 0x1C, "OBSTACLE")               \
    X(SURFACE_RUG, 0x1D, "RUG")                         \
    X(SURFACE_TREE, 0x1E, "TREE")                       \
    X(SURFACE_BOX, 0x1F, "BOX")                         \
    X(SURFACE_BRICK, 0x20, "BRICK")                     \
    X(SURFACE_CHIMNEY, 0x21, "CHIMNEY")                 \
    X(SURFACE_MUD_WALL, 0x22, "MUD_WALL")               \
    X(SURFACE_BRIDGE, 0x23, "BRIDGE")                   \
    X(SURFACE_WATER_PLANT, 0x24, "WATER_PLANT")         \
    X(SURFACE_STAIRS, 0x25, "STAIRS")                   \
    X(SURFACE_FURNITURE, 0x26, "FURNITURE")             \
    X(SURFACE_IVY, 0x27, "IVY")                         \
    X(SURFACE_DECK, 0x28, "DECK")                       \
    X(SURFACE_MACHINE, 0x29, "MACHINE")                 \
    X(SURFACE_IRON_PLATE, 0x2A, "IRON_PLATE")           \
    X(SURFACE_MOSS, 0x2B, "MOSS")                       \
    X(SURFACE_TOMBSTONE, 0x2C, "TOMBSTONE")             \
    X(SURFACE_WATERFALL, 0x2D, "WATERFALL")             \
    X(SURFACE_COFFIN, 0x2E, "COFFIN")                   \
    X(SURFACE_CROSS_SECTION, 0x3F, "CROSS_SECTION")

typedef enum {
#define X(oname, ovalue, ostring) oname = ovalue,
    SURFACE_INDEX
#undef X
} surface_e;

char* surface_str(surface_e value);

typedef enum {
    SLOPE_FLAT = 0x00,
    SLOPE_INCLINE_N = 0x85,
    SLOPE_INCLINE_E = 0x52,
    SLOPE_INCLINE_S = 0x25,
    SLOPE_INCLINE_W = 0x58,
    SLOPE_CONVEX_NE = 0x41,
    SLOPE_CONVEX_SE = 0x11,
    SLOPE_CONVEX_SW = 0x14,
    SLOPE_CONVEX_NW = 0x44,
    SLOPE_CONCAVE_NE = 0x96,
    SLOPE_CONCAVE_SE = 0x66,
    SLOPE_CONCAVE_SW = 0x69,
    SLOPE_CONCAVE_NW = 0x99,
} slope_e;

typedef struct {
    surface_e surface;
    slope_e slope;
    u8 sloped_height_bottom;
    u8 sloped_height_top; // difference between bottom and top
    u8 depth;
    u8 shading;
    u8 auto_cam_dir;    // auto rotate camera if unit enters this tile
    bool is_selectable; // Can walk/cursor but cannot stop on it
    bool is_walkable;
    bool is_cursorable;
} tile_t;

typedef struct {
    tile_t tiles[16][16]; // rows then columns
    bool valid;
} terrain_t;

terrain_t read_terrain(span_t*);
