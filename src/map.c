#include "map.h"
#include "game.h"
#include "texture.h"

void read_map(int num, map_state_t map_state, map_t* map_out)
{
    map_data_t* map_data = calloc(1, sizeof(map_data_t));
    read_map_data(num, map_data);

    map_out->map_state = map_state;
    map_out->map_data = map_data;

    if (map_data->primary_mesh.valid) {
        map_out->mesh = map_data->primary_mesh;
    } else {
        map_out->mesh = map_data->override_mesh;
    }

    for (int i = 0; i < map_data->alt_mesh_count; i++) {
        mesh_t alt_mesh = map_data->alt_meshes[i];
        if (alt_mesh.valid && map_state_eq(alt_mesh.map_state, map_state)) {
            merge_meshes(&map_out->mesh, &alt_mesh);
            break;
        }
    }

    for (int i = 0; i < map_data->texture_count; i++) {
        texture_t texture = map_data->textures[i];

        if (texture.valid && map_state_eq(texture.map_state, map_state)) {
            map_out->texture = texture;
            break;
        }
        if (texture.valid && map_state_default(texture.map_state)) {
            if (!map_out->texture.valid) {
                map_out->texture = texture;
            }
        }
    }

    map_out->vertices = geometry_to_vertices(&map_out->mesh.geometry);
    map_out->centered_translation = vertices_centered(&map_out->vertices);

    assert(map_out->mesh.valid);
    assert(map_out->texture.valid);
}

void read_map_data(int num, map_data_t* map_data_out)
{
    file_t gns_file = read_file(g.bin, map_list[num].sector, GNS_FILE_MAX_SIZE);
    map_data_out->record_count = read_records(&gns_file, map_data_out->records);

    free(gns_file.data);

    for (int i = 0; i < map_data_out->record_count; i++) {
        record_t record = map_data_out->records[i];
        file_t file = read_file(g.bin, record.sector, record.length);

        switch (record.type) {
        case FILETYPE_TEXTURE: {
            texture_t texture = read_texture(&file);
            texture.map_state = record.state;
            map_data_out->textures[map_data_out->texture_count++] = texture;
            break;
        }
        case FILETYPE_MESH_PRIMARY:
            // There always only one primary mesh file and it uses default state.
            assert(map_state_default(record.state));

            map_data_out->primary_mesh = read_mesh(&file);
            assert(map_data_out->primary_mesh.valid);
            break;

        case FILETYPE_MESH_ALT: {
            mesh_t alt_mesh = read_mesh(&file);
            alt_mesh.map_state = record.state;
            map_data_out->alt_meshes[map_data_out->alt_mesh_count++] = alt_mesh;
            break;
        }

        case FILETYPE_MESH_OVERRIDE: {
            // If there is an override file, there is only one and it uses default state.
            assert(map_state_default(record.state));

            map_data_out->override_mesh = read_mesh(&file);
            break;
        }

        default:
            free(file.data);
            assert(false);
        }

        free(file.data);
    }
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
