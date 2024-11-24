#ifndef MAP_H_
#define MAP_H_

#include "cglm/struct.h"

#include "map_record.h"
#include "mesh.h"
#include "texture.h"

#define MAP_COUNT 128

// map_data_t represents all the data for a specific map.
// This has all the different states a map can have.
typedef struct {
    record_t records[GNS_RECORD_MAX_NUM];

    mesh_t primary_mesh;
    mesh_t override_mesh;
    mesh_t alt_meshes[20];
    texture_t textures[20];

    int record_count;
    int texture_count;
    int alt_mesh_count;
} map_data_t;

// map_t represents map data for a specific map state.
// The map_data is all possible data for a map.
// The mesh and texture are the data for the specific state.
// The vertices are the mesh data converted to vertices for the shader.
typedef struct {
    map_state_t map_state;
    map_data_t* map_data;

    mesh_t mesh;
    texture_t texture;
    vertices_t vertices;
    vec3s centered_translation;
} map_t;

typedef struct {
    uint8_t id;
    uint16_t sector;
    bool valid;
    const char* name;
} map_desc_t;

void read_map(int, map_state_t, map_t*);
void read_map_data(int num, map_data_t* map_data_out);

extern map_desc_t map_list[];

#endif // MAP_H_
