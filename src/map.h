#pragma once

#include "defines.h"
#include "filesystem.h"
#include "image.h"
#include "map_record.h"
#include "mesh.h"

#define MAP_COUNT 128

// map_t is a struct that contains all the data for a map for all scenarios.
typedef struct {
    map_record_t records[MAP_RECORD_MAX_NUM];

    mesh_t primary_mesh;
    mesh_t override_mesh;
    mesh_t alt_meshes[20];
    image_t textures[20];

    int record_count;
    int texture_count;
    int alt_mesh_count;
} map_t;

map_t* read_map(int);
void map_destroy(map_t*);

// map_desc_t is a struct that contains information about a map.
// This lets us know if we can use the map and where on the disk it is.
typedef struct {
    u8 id;
    file_entry_e file;
    bool valid;
    const char* name;
} map_desc_t;

extern map_desc_t map_list[];
