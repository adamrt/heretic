#pragma once

#include "defines.h"
#include "span.h"

enum {
    TERRAIN_LEVEL_COUNT = 2,
    TERRAIN_X_MAX = 17,
    TERRAIN_Z_MAX = 18,
    TERRAIN_TILE_MAX = 256,

    TILE_WIDTH = 28,
    TILE_DEPTH = 28,
    TILE_HEIGHT = 12,

    UNIT_HEIGHT = TILE_HEIGHT * 3, // Regular unit

    TERRAIN_STR_SIZE = 128
};

#define SURFACE_INDEX                                   \
    X(SURFACE_NATURAL_SURFACE, 0x00, "Natural Surface") \
    X(SURFACE_SAND, 0x01, "Sand")                       \
    X(SURFACE_STALACTITE, 0x02, "Stalactite")           \
    X(SURFACE_GRASSLAND, 0x03, "Grassland")             \
    X(SURFACE_THICKET, 0x04, "Thicket")                 \
    X(SURFACE_SNOW, 0x05, "Snow")                       \
    X(SURFACE_ROCKY_CLIFF, 0x06, "Rocky Cliff")         \
    X(SURFACE_GRAVEL, 0x07, "Gravel")                   \
    X(SURFACE_WASTELAND, 0x08, "Wasteland")             \
    X(SURFACE_SWAMP, 0x09, "Swamp")                     \
    X(SURFACE_MARSH, 0x0A, "Marsh")                     \
    X(SURFACE_POISONED_MARSH, 0x0B, "Poisoned Marsh")   \
    X(SURFACE_LAVA_ROCKS, 0x0C, "Lava Rocks")           \
    X(SURFACE_ICE, 0x0D, "Ice")                         \
    X(SURFACE_WATERWAY, 0x0E, "Waterway")               \
    X(SURFACE_RIVER, 0x0F, "River")                     \
    X(SURFACE_LAKE, 0x10, "Lake")                       \
    X(SURFACE_SEA, 0x11, "Sea")                         \
    X(SURFACE_LAVA, 0x12, "Lava")                       \
    X(SURFACE_ROAD, 0x13, "Road")                       \
    X(SURFACE_WOODEN_FLOOR, 0x14, "Wooden Floor")       \
    X(SURFACE_STONE_FLOOR, 0x15, "Stone Floor")         \
    X(SURFACE_ROOF, 0x16, "Roof")                       \
    X(SURFACE_STONEWALL, 0x17, "Stonewall")             \
    X(SURFACE_SKY, 0x18, "Sky")                         \
    X(SURFACE_DARKNESS, 0x19, "Darkness")               \
    X(SURFACE_SALT, 0x1A, "Salt")                       \
    X(SURFACE_BOOK, 0x1B, "Book")                       \
    X(SURFACE_OBSTACLE, 0x1C, "Obstacle")               \
    X(SURFACE_RUG, 0x1D, "Rug")                         \
    X(SURFACE_TREE, 0x1E, "Tree")                       \
    X(SURFACE_BOX, 0x1F, "Box")                         \
    X(SURFACE_BRICK, 0x20, "Brick")                     \
    X(SURFACE_CHIMNEY, 0x21, "Chimney")                 \
    X(SURFACE_MUD_WALL, 0x22, "Mud Wall")               \
    X(SURFACE_BRIDGE, 0x23, "Bridge")                   \
    X(SURFACE_WATER_PLANT, 0x24, "Water Plant")         \
    X(SURFACE_STAIRS, 0x25, "Stairs")                   \
    X(SURFACE_FURNITURE, 0x26, "Furniture")             \
    X(SURFACE_IVY, 0x27, "Ivy")                         \
    X(SURFACE_DECK, 0x28, "Deck")                       \
    X(SURFACE_MACHINE, 0x29, "Machine")                 \
    X(SURFACE_IRON_PLATE, 0x2A, "Iron Plate")           \
    X(SURFACE_MOSS, 0x2B, "Moss")                       \
    X(SURFACE_TOMBSTONE, 0x2C, "Tombstone")             \
    X(SURFACE_WATERFALL, 0x2D, "Waterfall")             \
    X(SURFACE_COFFIN, 0x2E, "Coffin")                   \
    X(SURFACE_CROSS_SECTION, 0x3F, "Cross Section")

typedef enum {
#define X(oname, ovalue, ostring) oname = ovalue,
    SURFACE_INDEX
#undef X
} surface_e;

#define SLOPE_INDEX                         \
    X(SLOPE_FLAT, 0x00, "Flat")             \
    X(SLOPE_INCLINE_N, 0x85, "Incline N")   \
    X(SLOPE_INCLINE_E, 0x52, "Incline E")   \
    X(SLOPE_INCLINE_S, 0x25, "Incline S")   \
    X(SLOPE_INCLINE_W, 0x58, "Incline W")   \
    X(SLOPE_CONVEX_NE, 0x41, "Convex NE")   \
    X(SLOPE_CONVEX_SE, 0x11, "Convex SE")   \
    X(SLOPE_CONVEX_SW, 0x14, "Convex SW")   \
    X(SLOPE_CONVEX_NW, 0x44, "Convex NW")   \
    X(SLOPE_CONCAVE_NE, 0x96, "Concave NE") \
    X(SLOPE_CONCAVE_SE, 0x66, "Concave SE") \
    X(SLOPE_CONCAVE_SW, 0x69, "Concave SW") \
    X(SLOPE_CONCAVE_NW, 0x99, "Concave NW")

typedef enum {
#define X(oname, ovalue, ostring) oname = ovalue,
    SLOPE_INDEX
#undef X
} slope_e;

typedef struct {
    surface_e surface;
    slope_e slope;
    u8 sloped_height_bottom;
    u8 sloped_height_top; // difference between bottom and top
    u8 depth;
    u8 shading;
    u8 auto_cam_dir;        // auto rotate camera if unit enters this tile
    bool pass_through_only; // Can walk/cursor but cannot stop on it
    bool cant_walk;
    bool cant_select;
} tile_t;

typedef struct {
    tile_t tiles[TERRAIN_LEVEL_COUNT][TERRAIN_TILE_MAX];
    u8 x_count;
    u8 z_count;
    bool valid;
} terrain_t;

terrain_t read_terrain(span_t*);
const char* terrain_surface_str(surface_e);
const char* terrain_slope_str(slope_e);
const char* terrain_shading_str(u8);
void terrain_camdir_str(u8, char[static TERRAIN_STR_SIZE]);
