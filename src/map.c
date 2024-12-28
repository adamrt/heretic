#include "gfx.h"
#include "sokol_gfx.h"

#include "io.h"
#include "map.h"
#include "memory.h"
#include "texture.h"
#include "util.h"

model_t map_make_model(const map_t* map, map_state_t map_state) {
    mesh_t final_mesh = { 0 };
    texture_t final_texture = { 0 };
    if (map->primary_mesh.valid) {
        final_mesh = map->primary_mesh;
    } else {
        final_mesh = map->override_mesh;
    }

    for (int i = 0; i < map->alt_mesh_count; i++) {
        mesh_t alt_mesh = map->alt_meshes[i];
        if (alt_mesh.valid && map_state_eq(alt_mesh.map_state, map_state)) {
            merge_meshes(&final_mesh, &alt_mesh);
            break;
        }
    }

    for (int i = 0; i < map->texture_count; i++) {
        texture_t texture = map->textures[i];

        if (texture.valid && map_state_eq(texture.map_state, map_state)) {
            final_texture = texture;
            break;
        }
        if (texture.valid && map_state_default(texture.map_state)) {
            if (!final_texture.valid) {
                final_texture = texture;
            }
        }
    }

    ASSERT(final_mesh.valid, "Map mesh is invalid");
    ASSERT(final_texture.valid, "Map texture is invalid");

    vertices_t vertices = geometry_to_vertices(&final_mesh.geometry);

    sg_buffer vbuf = sg_make_buffer(&(sg_buffer_desc) {
        .data = SG_RANGE(vertices),
        .label = "mesh-vertices",
    });

    sg_image texture = texture_to_sg_image(final_texture);
    sg_image palette = texture_to_sg_image(final_mesh.palette);

    vec3s centered_translation = vertices_centered(&vertices);

    model_t model = {
        .vertex_count = final_mesh.geometry.vertex_count,
        .lighting = final_mesh.lighting,
        .center = centered_translation,
        .transform = {
            .translation = centered_translation, // Centering for now as it seems better.
            .scale = { { 1.0f, 1.0f, 1.0f } },
        },
        .vbuf = vbuf,
        .texture = texture,
        .palette = palette,
    };
    return model;
}

void map_destroy(map_t* map) {
    if (map == NULL) {
        return;
    }
    for (int i = 0; i < map->texture_count; i++) {
        texture_destroy(map->textures[i]);
    }
    for (int i = 0; i < map->alt_mesh_count; i++) {
        texture_destroy(map->alt_meshes[i].palette);
    }
    texture_destroy(map->primary_mesh.palette);
    texture_destroy(map->override_mesh.palette);
    memory_free(map);
}

map_t* read_map(int num) {
    bytes_t gnsfile = io_read_file_bytes(map_list[num].sector, MAP_FILE_MAX_SIZE);
    span_t gnsspan = { gnsfile.data, MAP_FILE_MAX_SIZE, 0 };

    map_t* map = memory_allocate(sizeof(map_t));
    map->record_count = read_map_records(&gnsspan, map->records);

    for (int i = 0; i < map->record_count; i++) {
        map_record_t* record = &map->records[i];

        bytes_t file_contents = io_read_file_bytes(record->sector, record->length);
        span_t file = { file_contents.data, record->length, 0 };

        switch (record->type) {
        case FILETYPE_TEXTURE: {
            texture_t texture = read_texture(&file);
            texture.map_state = record->state;
            map->textures[map->texture_count++] = texture;
            break;
        }
        case FILETYPE_MESH_PRIMARY:
            // There always only one primary mesh file and it uses default state.
            ASSERT(map_state_default(record->state), "Primary mesh file has non-default state");

            map->primary_mesh = read_mesh(&file);
            record->vertex_count = map->primary_mesh.geometry.vertex_count;
            record->light_count = map->primary_mesh.lighting.light_count;
            record->valid_palette = map->primary_mesh.palette.valid;

            ASSERT(map->primary_mesh.valid, "Primary mesh is invalid");
            break;

        case FILETYPE_MESH_ALT: {
            mesh_t alt_mesh = read_mesh(&file);
            alt_mesh.map_state = record->state;
            map->alt_meshes[map->alt_mesh_count++] = alt_mesh;
            record->vertex_count = alt_mesh.geometry.vertex_count;
            record->light_count = alt_mesh.lighting.light_count;
            record->valid_palette = alt_mesh.palette.valid;
            break;
        }

        case FILETYPE_MESH_OVERRIDE: {
            // If there is an override file, there is only one and it uses default state.
            ASSERT(map_state_default(record->state), "Oerride must be default map state");

            map->override_mesh = read_mesh(&file);
            record->vertex_count = map->override_mesh.geometry.vertex_count;
            record->light_count = map->override_mesh.lighting.light_count;
            record->valid_palette = map->override_mesh.palette.valid;
            break;
        }

        default:
            ASSERT(false, "Unknown map file type");
        }
    }

    return map;
}

map_desc_t map_list[MAP_COUNT] = {
    { 0, 10026, false, "Unknown" }, // No texture
    { 1, 11304, true, "At Main Gate of Igros Castle" },
    { 2, 12656, true, "Back Gate of Lesalia Castle" },
    { 3, 12938, true, "Hall of St. Murond Temple" },
    { 4, 13570, true, "Office of Lesalia Castle" },
    { 5, 14239, true, "Roof of Riovanes Castle" },
    { 6, 14751, true, "At the Gate of Riovanes Castle" },
    { 7, 15030, true, "Inside of Riovanes Castle" },
    { 8, 15595, true, "Riovanes Castle" },
    { 9, 16262, true, "Citadel of Igros Castle" },
    { 10, 16347, true, "Inside of Igros Castle" },
    { 11, 16852, true, "Office of Igros Castle" },
    { 12, 17343, true, "At the Gate of Lionel Castle" },
    { 13, 17627, true, "Inside of Lionel Castle" },
    { 14, 18175, true, "Office of Lionel Castle" },
    { 15, 19510, true, "At the Gate of Limberry Castle (1)" },
    { 16, 20075, true, "Inside of Limberry Castle" },
    { 17, 20162, true, "Underground Cemetery of Limberry Castle" },
    { 18, 20745, true, "Office of Limberry Castle" },
    { 19, 21411, true, "At the Gate of Limberry Castle (2)" },
    { 20, 21692, true, "Inside of Zeltennia Castle" },
    { 21, 22270, true, "Zeltennia Castle" },
    { 22, 22938, true, "Magic City Gariland" },
    { 23, 23282, true, "Belouve Residence" },
    { 24, 23557, true, "Military Academy's Auditorium" },
    { 25, 23899, true, "Yardow Fort City" },
    { 26, 23988, true, "Weapon Storage of Yardow" },
    { 27, 24266, true, "Goland Coal City" },
    { 28, 24544, true, "Colliery Underground First Floor" },
    { 29, 24822, true, "Colliery Underground Second Floor" },
    { 30, 25099, true, "Colliery Underground Third Floor" },
    { 31, 25764, true, "Dorter Trade City" },
    { 32, 26042, true, "Slums in Dorter" },
    { 33, 26229, true, "Hospital in Slums" },
    { 34, 26362, true, "Cellar of Sand Mouse" },
    { 35, 27028, true, "Zaland Fort City" },
    { 36, 27643, true, "Church Outside of Town" },
    { 37, 27793, true, "Ruins Outside Zaland" },
    { 38, 28467, true, "Goug Machine City" },
    { 39, 28555, true, "Underground Passage in Goland" },
    { 40, 29165, true, "Slums in Goug" },
    { 41, 29311, true, "Besrodio's House" },
    { 42, 29653, true, "Warjilis Trade City" },
    { 43, 29807, true, "Port of Warjilis" },
    { 44, 30473, true, "Bervenia Free City" },
    { 45, 30622, true, "Ruins of Zeltennia Castle's Church" },
    { 46, 30966, true, "Cemetery of Heavenly Knight, Balbanes" },
    { 47, 31697, true, "Zarghidas Trade City" },
    { 48, 32365, true, "Slums of Zarghidas" },
    { 49, 33032, true, "Fort Zeakden" },
    { 50, 33701, true, "St. Murond Temple" },
    { 51, 34349, true, "St. Murond Temple" },
    { 52, 34440, true, "Chapel of St. Murond Temple" },
    { 53, 34566, true, "Entrance to Death City" },
    { 54, 34647, true, "Lost Sacred Precincts" },
    { 55, 34745, true, "Graveyard of Airships" },
    { 56, 35350, true, "Orbonne Monastery" },
    { 57, 35436, true, "Underground Book Storage First Floor" },
    { 58, 35519, true, "Underground Book Storage Second Floor" },
    { 59, 35603, true, "Underground Book Storage Third Floor" },
    { 60, 35683, true, "Underground Book Storage Fourth Floor" },
    { 61, 35765, true, "Underground Book Storage Fifth Floor" },
    { 62, 36052, true, "Chapel of Orbonne Monastery" },
    { 63, 36394, true, "Golgorand Execution Site" },
    { 64, 36530, true, "In Front of Bethla Garrison's Sluice" },
    { 65, 36612, true, "Granary of Bethla Garrison" },
    { 66, 37214, true, "South Wall of Bethla Garrison" },
    { 67, 37817, true, "North Wall of Bethla Garrison" },
    { 68, 38386, true, "Bethla Garrison" },
    { 69, 38473, true, "Murond Death City" },
    { 70, 38622, true, "Nelveska Temple" },
    { 71, 39288, true, "Dolbodar Swamp" },
    { 72, 39826, true, "Fovoham Plains" },
    { 73, 40120, true, "Inside of Windmill Shed" },
    { 74, 40724, true, "Sweegy Woods" },
    { 75, 41391, true, "Bervenia Volcano" },
    { 76, 41865, true, "Zeklaus Desert" },
    { 77, 42532, true, "Lenalia Plateau" },
    { 78, 43200, true, "Zigolis Swamp" },
    { 79, 43295, true, "Yuguo Woods" },
    { 80, 43901, true, "Araguay Woods" },
    { 81, 44569, true, "Grog Hill" },
    { 82, 45044, true, "Bed Desert" },
    { 83, 45164, true, "Zirekile Falls" },
    { 84, 45829, true, "Bariaus Hill" },
    { 85, 46498, true, "Mandalia Plains" },
    { 86, 47167, true, "Doguola Pass" },
    { 87, 47260, true, "Bariaus Valley" },
    { 88, 47928, true, "Finath River" },
    { 89, 48595, true, "Poeskas Lake" },
    { 90, 49260, true, "Germinas Peak" },
    { 91, 49538, true, "Thieves Fort" },
    { 92, 50108, true, "Igros-Belouve Residence" },
    { 93, 50387, true, "Broke Down Shed-Wooden Building" },
    { 94, 50554, true, "Broke Down Shed-Stone Building" },
    { 95, 51120, true, "Church" },
    { 96, 51416, true, "Pub" },
    { 97, 52082, true, "Inside Castle Gate in Lesalia" },
    { 98, 52749, true, "Outside Castle Gate in Lesalia" },
    { 99, 53414, true, "Main Street of Lesalia" },
    { 100, 53502, true, "Public Cemetery" },
    { 101, 53579, true, "Tutorial (1)" },
    { 102, 53659, true, "Tutorial (2)" },
    { 103, 54273, true, "Windmill Shed" },
    { 104, 54359, true, "Belouve Residence" },
    { 105, 54528, true, "TERMINATE" },
    { 106, 54621, true, "DELTA" },
    { 107, 54716, true, "NOGIAS" },
    { 108, 54812, true, "VOYAGE" },
    { 109, 54909, true, "BRIDGE" },
    { 110, 55004, true, "VALKYRIES" },
    { 111, 55097, true, "MLAPAN" },
    { 112, 55192, true, "TIGER" },
    { 113, 55286, true, "HORROR" },
    { 114, 55383, true, "END" },
    { 115, 56051, true, "Banished Fort" },
    { 116, 56123, true, "Arena" },
    { 117, 56201, true, "Unknown" },
    { 118, 56279, true, "Unknown" },
    { 119, 56356, true, "Unknown" },
    { 120, 0, false, "???" },
    { 121, 0, false, "???" },
    { 122, 0, false, "???" },
    { 123, 0, false, "???" },
    { 124, 0, false, "???" },
    { 125, 56435, true, "Unknown" },
    { 126, 0, false, "???" },
    { 127, 0, false, "???" },
};
