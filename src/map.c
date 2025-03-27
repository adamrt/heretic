#include "sokol_gfx.h"

#include "filesystem.h"
#include "image.h"
#include "map.h"
#include "memory.h"
#include "texture.h"
#include "util.h"

static image_t _read_map_texture(span_t* span) {
    constexpr int width = 256;
    constexpr int height = 1024;
    return image_read_4bpp(span, width, height);
}

model_t map_make_model(const map_t* map, map_state_t map_state) {
    mesh_t final_mesh = {};
    image_t final_texture = {};
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
        image_t texture = map->textures[i];

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

    texture_t texture = texture_create(final_texture);
    texture_t palette = texture_create(final_mesh.palette);

    vec3s centered_translation = vertices_centered(&vertices);

    model_t model = {
        .vertex_count = final_mesh.geometry.vertex_count,
        .lighting = final_mesh.lighting,
        .center = centered_translation,
        .transform.scale = { { 1.0f, 1.0f, 1.0f } },
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

    // Textures
    for (int i = 0; i < map->texture_count; i++) {
        image_destroy(map->textures[i]);
    }

    // Palettes
    image_destroy(map->primary_mesh.palette);
    image_destroy(map->override_mesh.palette);
    for (int i = 0; i < map->alt_mesh_count; i++) {
        image_destroy(map->alt_meshes[i].palette);
    }

    memory_free(map);
}

map_t* read_map(int num) {

    // Fetch the GNS file which contains pointers to the map's resources.
    const file_entry_e map_file = map_list[num].file;
    span_t gnsspan = filesystem_read_file(map_file);

    map_t* map = memory_allocate(sizeof(map_t));

    map->record_count = read_map_records(&gnsspan, map->records);

    for (int i = 0; i < map->record_count; i++) {
        map_record_t* record = &map->records[i];

        // Fetch the resource file
        const file_entry_e entry = filesystem_entry_by_sector(record->sector);
        span_t file = filesystem_read_file(entry);

        switch (record->type) {
        case FILETYPE_TEXTURE: {
            image_t texture = _read_map_texture(&file);
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
            record->valid_terrain = map->primary_mesh.terrain.valid;

            ASSERT(map->primary_mesh.valid, "Primary mesh is invalid");
            break;

        case FILETYPE_MESH_ALT: {
            mesh_t alt_mesh = read_mesh(&file);
            alt_mesh.map_state = record->state;
            map->alt_meshes[map->alt_mesh_count++] = alt_mesh;
            record->vertex_count = alt_mesh.geometry.vertex_count;
            record->light_count = alt_mesh.lighting.light_count;
            record->valid_palette = alt_mesh.palette.valid;
            record->valid_terrain = alt_mesh.terrain.valid;
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
    { 0, F_MAP__MAP000_GNS, false, "Unknown" }, // No texture
    { 1, F_MAP__MAP001_GNS, true, "At Main Gate of Igros Castle" },
    { 2, F_MAP__MAP002_GNS, true, "Back Gate of Lesalia Castle" },
    { 3, F_MAP__MAP003_GNS, true, "Hall of St. Murond Temple" },
    { 4, F_MAP__MAP004_GNS, true, "Office of Lesalia Castle" },
    { 5, F_MAP__MAP005_GNS, true, "Roof of Riovanes Castle" },
    { 6, F_MAP__MAP006_GNS, true, "At the Gate of Riovanes Castle" },
    { 7, F_MAP__MAP007_GNS, true, "Inside of Riovanes Castle" },
    { 8, F_MAP__MAP008_GNS, true, "Riovanes Castle" },
    { 9, F_MAP__MAP009_GNS, true, "Citadel of Igros Castle" },
    { 10, F_MAP__MAP010_GNS, true, "Inside of Igros Castle" },
    { 11, F_MAP__MAP011_GNS, true, "Office of Igros Castle" },
    { 12, F_MAP__MAP012_GNS, true, "At the Gate of Lionel Castle" },
    { 13, F_MAP__MAP013_GNS, true, "Inside of Lionel Castle" },
    { 14, F_MAP__MAP014_GNS, true, "Office of Lionel Castle" },
    { 15, F_MAP__MAP015_GNS, true, "At the Gate of Limberry Castle (1)" },
    { 16, F_MAP__MAP016_GNS, true, "Inside of Limberry Castle" },
    { 17, F_MAP__MAP017_GNS, true, "Underground Cemetery of Limberry Castle" },
    { 18, F_MAP__MAP018_GNS, true, "Office of Limberry Castle" },
    { 19, F_MAP__MAP019_GNS, true, "At the Gate of Limberry Castle (2)" },
    { 20, F_MAP__MAP020_GNS, true, "Inside of Zeltennia Castle" },
    { 21, F_MAP__MAP021_GNS, true, "Zeltennia Castle" },
    { 22, F_MAP__MAP022_GNS, true, "Magic City Gariland" },
    { 23, F_MAP__MAP023_GNS, true, "Belouve Residence" },
    { 24, F_MAP__MAP024_GNS, true, "Military Academy's Auditorium" },
    { 25, F_MAP__MAP025_GNS, true, "Yardow Fort City" },
    { 26, F_MAP__MAP026_GNS, true, "Weapon Storage of Yardow" },
    { 27, F_MAP__MAP027_GNS, true, "Goland Coal City" },
    { 28, F_MAP__MAP028_GNS, true, "Colliery Underground First Floor" },
    { 29, F_MAP__MAP029_GNS, true, "Colliery Underground Second Floor" },
    { 30, F_MAP__MAP030_GNS, true, "Colliery Underground Third Floor" },
    { 31, F_MAP__MAP031_GNS, true, "Dorter Trade City" },
    { 32, F_MAP__MAP032_GNS, true, "Slums in Dorter" },
    { 33, F_MAP__MAP033_GNS, true, "Hospital in Slums" },
    { 34, F_MAP__MAP034_GNS, true, "Cellar of Sand Mouse" },
    { 35, F_MAP__MAP035_GNS, true, "Zaland Fort City" },
    { 36, F_MAP__MAP036_GNS, true, "Church Outside of Town" },
    { 37, F_MAP__MAP037_GNS, true, "Ruins Outside Zaland" },
    { 38, F_MAP__MAP038_GNS, true, "Goug Machine City" },
    { 39, F_MAP__MAP039_GNS, true, "Underground Passage in Goland" },
    { 40, F_MAP__MAP040_GNS, true, "Slums in Goug" },
    { 41, F_MAP__MAP041_GNS, true, "Besrodio's House" },
    { 42, F_MAP__MAP042_GNS, true, "Warjilis Trade City" },
    { 43, F_MAP__MAP043_GNS, true, "Port of Warjilis" },
    { 44, F_MAP__MAP044_GNS, true, "Bervenia Free City" },
    { 45, F_MAP__MAP045_GNS, true, "Ruins of Zeltennia Castle's Church" },
    { 46, F_MAP__MAP046_GNS, true, "Cemetery of Heavenly Knight, Balbanes" },
    { 47, F_MAP__MAP047_GNS, true, "Zarghidas Trade City" },
    { 48, F_MAP__MAP048_GNS, true, "Slums of Zarghidas" },
    { 49, F_MAP__MAP049_GNS, true, "Fort Zeakden" },
    { 50, F_MAP__MAP050_GNS, true, "St. Murond Temple" },
    { 51, F_MAP__MAP051_GNS, true, "St. Murond Temple" },
    { 52, F_MAP__MAP052_GNS, true, "Chapel of St. Murond Temple" },
    { 53, F_MAP__MAP053_GNS, true, "Entrance to Death City" },
    { 54, F_MAP__MAP054_GNS, true, "Lost Sacred Precincts" },
    { 55, F_MAP__MAP055_GNS, true, "Graveyard of Airships" },
    { 56, F_MAP__MAP056_GNS, true, "Orbonne Monastery" },
    { 57, F_MAP__MAP057_GNS, true, "Underground Book Storage First Floor" },
    { 58, F_MAP__MAP058_GNS, true, "Underground Book Storage Second Floor" },
    { 59, F_MAP__MAP059_GNS, true, "Underground Book Storage Third Floor" },
    { 60, F_MAP__MAP060_GNS, true, "Underground Book Storage Fourth Floor" },
    { 61, F_MAP__MAP061_GNS, true, "Underground Book Storage Fifth Floor" },
    { 62, F_MAP__MAP062_GNS, true, "Chapel of Orbonne Monastery" },
    { 63, F_MAP__MAP063_GNS, true, "Golgorand Execution Site" },
    { 64, F_MAP__MAP064_GNS, true, "In Front of Bethla Garrison's Sluice" },
    { 65, F_MAP__MAP065_GNS, true, "Granary of Bethla Garrison" },
    { 66, F_MAP__MAP066_GNS, true, "South Wall of Bethla Garrison" },
    { 67, F_MAP__MAP067_GNS, true, "North Wall of Bethla Garrison" },
    { 68, F_MAP__MAP068_GNS, true, "Bethla Garrison" },
    { 69, F_MAP__MAP069_GNS, true, "Murond Death City" },
    { 70, F_MAP__MAP070_GNS, true, "Nelveska Temple" },
    { 71, F_MAP__MAP071_GNS, true, "Dolbodar Swamp" },
    { 72, F_MAP__MAP072_GNS, true, "Fovoham Plains" },
    { 73, F_MAP__MAP073_GNS, true, "Inside of Windmill Shed" },
    { 74, F_MAP__MAP074_GNS, true, "Sweegy Woods" },
    { 75, F_MAP__MAP075_GNS, true, "Bervenia Volcano" },
    { 76, F_MAP__MAP076_GNS, true, "Zeklaus Desert" },
    { 77, F_MAP__MAP077_GNS, true, "Lenalia Plateau" },
    { 78, F_MAP__MAP078_GNS, true, "Zigolis Swamp" },
    { 79, F_MAP__MAP079_GNS, true, "Yuguo Woods" },
    { 80, F_MAP__MAP080_GNS, true, "Araguay Woods" },
    { 81, F_MAP__MAP081_GNS, true, "Grog Hill" },
    { 82, F_MAP__MAP082_GNS, true, "Bed Desert" },
    { 83, F_MAP__MAP083_GNS, true, "Zirekile Falls" },
    { 84, F_MAP__MAP084_GNS, true, "Bariaus Hill" },
    { 85, F_MAP__MAP085_GNS, true, "Mandalia Plains" },
    { 86, F_MAP__MAP086_GNS, true, "Doguola Pass" },
    { 87, F_MAP__MAP087_GNS, true, "Bariaus Valley" },
    { 88, F_MAP__MAP088_GNS, true, "Finath River" },
    { 89, F_MAP__MAP089_GNS, true, "Poeskas Lake" },
    { 90, F_MAP__MAP090_GNS, true, "Germinas Peak" },
    { 91, F_MAP__MAP091_GNS, true, "Thieves Fort" },
    { 92, F_MAP__MAP092_GNS, true, "Igros-Belouve Residence" },
    { 93, F_MAP__MAP093_GNS, true, "Broke Down Shed-Wooden Building" },
    { 94, F_MAP__MAP094_GNS, true, "Broke Down Shed-Stone Building" },
    { 95, F_MAP__MAP095_GNS, true, "Church" },
    { 96, F_MAP__MAP096_GNS, true, "Pub" },
    { 97, F_MAP__MAP097_GNS, true, "Inside Castle Gate in Lesalia" },
    { 98, F_MAP__MAP098_GNS, true, "Outside Castle Gate in Lesalia" },
    { 99, F_MAP__MAP099_GNS, true, "Main Street of Lesalia" },
    { 100, F_MAP__MAP100_GNS, true, "Public Cemetery" },
    { 101, F_MAP__MAP101_GNS, true, "Tutorial (1)" },
    { 102, F_MAP__MAP102_GNS, true, "Tutorial (2)" },
    { 103, F_MAP__MAP103_GNS, true, "Windmill Shed" },
    { 104, F_MAP__MAP104_GNS, true, "Belouve Residence" },
    { 105, F_MAP__MAP105_GNS, true, "TERMINATE" },
    { 106, F_MAP__MAP106_GNS, true, "DELTA" },
    { 107, F_MAP__MAP107_GNS, true, "NOGIAS" },
    { 108, F_MAP__MAP108_GNS, true, "VOYAGE" },
    { 109, F_MAP__MAP109_GNS, true, "BRIDGE" },
    { 110, F_MAP__MAP110_GNS, true, "VALKYRIES" },
    { 111, F_MAP__MAP111_GNS, true, "MLAPAN" },
    { 112, F_MAP__MAP112_GNS, true, "TIGER" },
    { 113, F_MAP__MAP113_GNS, true, "HORROR" },
    { 114, F_MAP__MAP114_GNS, true, "END" },
    { 115, F_MAP__MAP115_GNS, true, "Banished Fort" },
    { 116, F_MAP__MAP116_GNS, true, "Arena" },
    { 117, F_MAP__MAP117_GNS, true, "Unknown" },
    { 118, F_MAP__MAP118_GNS, true, "Unknown" },
    { 119, F_MAP__MAP119_GNS, true, "Unknown" },
    { 120, 0, false, "???" },
    { 121, 0, false, "???" },
    { 122, 0, false, "???" },
    { 123, 0, false, "???" },
    { 124, 0, false, "???" },
    { 125, F_MAP__MAP125_GNS, true, "Unknown" },
    { 126, 0, false, "???" },
    { 127, 0, false, "???" },
};
