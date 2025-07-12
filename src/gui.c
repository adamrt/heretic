#include <string.h>

#include "cglm/util.h"
#include "cimgui.h"
#include "image.h"
#include "sokol_app.h"
#include "sokol_gfx.h"
#include "sokol_glue.h"
#include "sokol_imgui.h"
#include "sokol_log.h"

#include "camera.h"
#include "event.h"
#include "filesystem.h"
#include "font.h"
#include "gfx.h"
#include "gfx_model.h"
#include "gfx_sprite.h"
#include "gui.h"
#include "map.h"
#include "map_record.h"
#include "memory.h"
#include "message.h"
#include "opcode.h"
#include "parse.h"
#include "scene.h"
#include "unit.h"
#include "util.h"
#include "vm.h"

static void _draw(void);
static uint32_t hash_int_rand_color(u32 v);
static uint32_t hash_map_state_rand_color(map_state_t state);

static struct {
    bool show_sprite_window[F_FILE_COUNT];
    u8 current_palette_idx[F_FILE_COUNT];

    bool show_window_scene;

    bool show_window_map_lights;
    bool show_window_map_records;
    bool show_window_raw_records;

    bool show_window_event_text;
    bool show_window_event_instructions;
    bool show_window_event_units;

    bool show_texture_resources;
    bool show_window_terrain;
    bool show_window_mesh;

    bool show_window_demo;
} _state;

static bool is_hovered = false;

void gui_init(void) {
    simgui_setup(&(simgui_desc_t) {
        .logger.func = slog_func,
    });

    ImGuiIO* io = igGetIO();
    io->ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io->IniFilename = "imgui.ini";

    _state.show_window_scene = true;
    _state.show_window_map_lights = true;
    _state.show_window_map_records = true;
    _state.show_window_event_text = true;
    _state.show_window_event_instructions = true;
    _state.show_window_event_units = true;
    _state.show_window_raw_records = true;
    _state.show_texture_resources = false;
    _state.show_window_demo = false;
    _state.show_window_terrain = true;
    _state.show_window_mesh = true;
}

void gui_shutdown(void) {
    igSaveIniSettingsToDisk("imgui.ini");
    simgui_shutdown();
}

void gui_update(void) {
    simgui_new_frame(&(simgui_frame_desc_t) {
        .width = sapp_width(),
        .height = sapp_height(),
        .delta_time = sapp_frame_duration(),
        .dpi_scale = sapp_dpi_scale(),
    });

    _draw();

    simgui_render();
}

bool gui_input(const sapp_event* event) {

    bool is_handled = simgui_handle_event(event);
    if (is_hovered) {
        return false;
    }
    return is_handled;
}

static void _draw_window_map_records(void) {
    scene_t* scene = scene_get_internals();
    igBegin("Records", &_state.show_window_map_records, 0);
    igCheckbox("Show Texture Resources", &_state.show_texture_resources);

    if (igBeginTable("", 9, ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_BordersOuterV | ImGuiTableFlags_RowBg)) {
        igTableSetupColumnEx("Action", ImGuiTableColumnFlags_WidthFixed, 50.0f, 0);
        igTableSetupColumnEx("Layout", ImGuiTableColumnFlags_WidthFixed, 25.0f, 0);
        igTableSetupColumnEx("Time", ImGuiTableColumnFlags_WidthStretch, 50.0f, 0);
        igTableSetupColumnEx("Weather", ImGuiTableColumnFlags_WidthStretch, 75.0f, 0);
        igTableSetupColumnEx("Type", ImGuiTableColumnFlags_WidthStretch, 75.0f, 0);
        igTableSetupColumnEx("Verts", ImGuiTableColumnFlags_WidthStretch, 50.0f, 0);
        igTableSetupColumnEx("Lights", ImGuiTableColumnFlags_WidthStretch, 50.0f, 0);
        igTableSetupColumnEx("Pallete", ImGuiTableColumnFlags_WidthStretch, 50.0f, 0);
        igTableSetupColumnEx("Terrain", ImGuiTableColumnFlags_WidthStretch, 50.0f, 0);

        igTableHeadersRow();

        for (int i = 0; i < scene->map->record_count; i++) {
            map_record_t r = scene->map->records[i];
            if (!_state.show_texture_resources && r.type == FILETYPE_TEXTURE) {
                continue;
            }

            igTableNextRow();
            igTableSetColumnIndex(0);
            igPushIDInt(i);
            if (igButton("Load")) {
                scene_load_map(scene->current_map, r.state);
            }
            igPopID();
            igTableSetColumnIndex(1);
            igText("%d", r.state.layout);
            igTableSetColumnIndex(2);
            igText("%s", time_str(r.state.time));
            igTableSetColumnIndex(3);
            igText("%s", weather_str(r.state.weather));
            igTableSetColumnIndex(4);
            igText("%s", filetype_str(r.type));
            if (r.vertex_count > 0) {
                igTableSetColumnIndex(5);
                igText("%d", r.vertex_count);
            }
            if (r.light_count > 0) {
                igTableSetColumnIndex(6);
                igText("%d", r.light_count);
            }
            if (r.valid_palette) {
                igTableSetColumnIndex(7);
                igText("true");
            }
            if (r.valid_terrain) {
                igTableSetColumnIndex(8);
                igText("true");
            }
        }
        igEndTable();
    }
    igEnd();
}

static void _draw_window_raw_records(void) {
    scene_t* scene = scene_get_internals();
    igBegin("Raw Records", &_state.show_window_raw_records, 0);
    if (igBeginTable("", 2, ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_BordersOuterV | ImGuiTableFlags_RowBg)) {
        igTableSetupColumnEx("Type", ImGuiTableColumnFlags_WidthFixed, 50.0f, 0);
        igTableSetupColumnEx("Raw Data", ImGuiTableColumnFlags_WidthStretch, 0.0f, 0);
        igTableHeadersRow();

        for (int i = 0; i < scene->map->record_count; i++) {
            map_record_t r = scene->map->records[i];
            ImU32 bg_color = hash_map_state_rand_color(r.state);
            igTableNextRow();
            igTableSetBgColor(ImGuiTableBgTarget_RowBg0, bg_color, -1);
            igTableSetColumnIndex(0);
            igText("%s", filetype_str(r.type));
            igTableSetColumnIndex(1);
            for (int j = 0; j < 20; j += 2) {
                uint16_t word = r.data[j + 1] | (r.data[j] << 8);
                igSameLine();
                igText("%04X", word);
            }
        }
        igEndTable();
    }
    igEnd();
}

static void _draw_window_mesh(void) {
    scene_t* scene = scene_get_internals();
    igBegin("Mesh", &_state.show_window_mesh, 0);
    if (igBeginTable("Vertices", 4, ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_BordersOuterV | ImGuiTableFlags_RowBg)) {
        igTableSetupColumnEx("Poly#", ImGuiTableColumnFlags_WidthFixed, 100.0f, 0);
        igTableSetupColumnEx("X", ImGuiTableColumnFlags_WidthFixed, 50.0f, 0);
        igTableSetupColumnEx("Y", ImGuiTableColumnFlags_WidthFixed, 50.0f, 0);
        igTableSetupColumnEx("Z", ImGuiTableColumnFlags_WidthFixed, 50.0f, 0);

        igTableHeadersRow();

        for (int i = 0; i < scene->map->primary_mesh.geometry.tex_quad_count; i++) {
            quad_t quad = scene->map->primary_mesh.geometry.tex_quads[i];
            for (int j = 0; j < 4; j++) {
                igTableNextRow();
                u32 bg_color = hash_int_rand_color(i);
                igTableSetBgColor(ImGuiTableBgTarget_RowBg0, bg_color, -1);
                igTableSetColumnIndex(0);
                igText("Tex Quad %d", i);
                igTableSetColumnIndex(1);
                igText("%0.0f", quad.vertices[j].position.x);
                igTableSetColumnIndex(2);
                igText("%0.0f", quad.vertices[j].position.y);
                igTableSetColumnIndex(3);
                igText("%0.0f", quad.vertices[j].position.z);
            }
        }

        for (int i = 0; i < scene->map->primary_mesh.geometry.untex_quad_count; i++) {
            quad_t quad = scene->map->primary_mesh.geometry.untex_quads[i];
            for (int j = 0; j < 4; j++) {
                igTableNextRow();
                u32 bg_color = hash_int_rand_color(i);
                igTableSetBgColor(ImGuiTableBgTarget_RowBg0, bg_color, -1);
                igTableSetColumnIndex(0);
                igText("Untex Quad %d", i);
                igTableSetColumnIndex(1);
                igText("%0.0f", quad.vertices[j].position.x);
                igTableSetColumnIndex(2);
                igText("%0.0f", quad.vertices[j].position.y);
                igTableSetColumnIndex(3);
                igText("%0.0f", quad.vertices[j].position.z);
            }
        }

        for (int i = 0; i < scene->map->primary_mesh.geometry.tex_tri_count; i++) {
            triangle_t tri = scene->map->primary_mesh.geometry.tex_tris[i];
            for (int j = 0; j < 3; j++) {
                igTableNextRow();
                u32 bg_color = hash_int_rand_color(i);
                igTableSetBgColor(ImGuiTableBgTarget_RowBg0, bg_color, -1);
                igTableSetColumnIndex(0);
                igText("Tex Tri %d", i);
                igTableSetColumnIndex(1);
                igText("%0.0f", tri.vertices[j].position.x);
                igTableSetColumnIndex(2);
                igText("%0.0f", tri.vertices[j].position.y);
                igTableSetColumnIndex(3);
                igText("%0.0f", tri.vertices[j].position.z);
            }
        }

        for (int i = 0; i < scene->map->primary_mesh.geometry.tex_tri_count; i++) {
            triangle_t tri = scene->map->primary_mesh.geometry.tex_tris[i];
            for (int j = 0; j < 3; j++) {
                igTableNextRow();
                u32 bg_color = hash_int_rand_color(i);
                igTableSetBgColor(ImGuiTableBgTarget_RowBg0, bg_color, -1);
                igTableSetColumnIndex(0);
                igText("Untex Tri %d", i);
                igTableSetColumnIndex(1);
                igText("%0.0f", tri.vertices[j].position.x);
                igTableSetColumnIndex(2);
                igText("%0.0f", tri.vertices[j].position.y);
                igTableSetColumnIndex(3);
                igText("%0.0f", tri.vertices[j].position.z);
            }
        }
        igEndTable();
    }
    igEnd();
}

static void _draw_window_sprite_font(void) {
    igBegin("FONT.BIN", &_state.show_sprite_window[F_EVENT__FONT_BIN], 0);
    ImVec2 dims = { FONT_ATLAS_WIDTH, FONT_ATLAS_HEIGHT };
    igImage(simgui_imtextureid(font_get_atlas_image()), dims);
    igEnd();
}

static void _draw_window_sprite_paletted(file_entry_e entry, int width, int height) {
    image_desc_t image_desc = image_get_desc(entry);

    igBegin(image_desc.name, &_state.show_sprite_window[entry], 0);

    ImVec2 dims = { width * 2, height * 2 };
    u8* selected = &_state.current_palette_idx[entry];

    char buf[16];
    snprintf(buf, sizeof(buf), "Palette %d", *selected);
    if (igBeginCombo("Palette##combo", buf, 0)) {
        for (int i = 0; i < image_desc.pal_count; i++) {
            bool is_selected = (*selected == i);

            snprintf(buf, sizeof(buf), "Palette %d", i);
            if (igSelectable(buf)) {
                *selected = i;
            }

            if (is_selected)
                igSetItemDefaultFocus();
        }
        igEndCombo();
    }

    igSeparator();

    texture_t texture = sprite_get_paletted_texture(entry, *selected);
    igImage(texture_imgui_id(texture), dims);
    igEnd();
}

static void _draw_window_event_text(void) {
    scene_t* scene = scene_get_internals();
    igBegin("Event Text", &_state.show_window_event_text, 0);
    if (scene->event.message_count == 0) {
        igText("No messages");
        igEnd();
        return;
    }
    if (igBeginTable("", 1, ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_BordersOuterV | ImGuiTableFlags_RowBg)) {
        igTableSetupColumnEx("Message", ImGuiTableColumnFlags_WidthStretch, 0.0f, 0);
        for (int i = 0; i < scene->event.message_count; i++) {
            igTableNextRow();
            igTableSetColumnIndex(0);
            char text[4096];
            message_by_index(scene->event.messages, i + 1, text);
            igText("%s", text);
        }
        igEndTable();
    }
    igEnd();
}

static void _draw_window_scene(void) {
    camera_t* cam = camera_get_internals();
    scene_t* scene = scene_get_internals();
    igBegin("Scene", &_state.show_window_scene, 0);
    event_desc_t desc = event_get_desc_by_scenario_id(scene->current_scenario_id);

    if (igRadioButton("Events", scene->mode == MODE_EVENT)) {
        scene->mode = MODE_EVENT;
        _state.show_window_event_instructions = true;
        _state.show_window_event_units = true;
        _state.show_window_event_text = true;
        _state.show_window_map_lights = false;
        _state.show_window_map_records = false;
    }
    igSameLine();
    if (igRadioButton("Maps", scene->mode == MODE_MAP)) {
        scene->mode = MODE_MAP;
        _state.show_window_event_instructions = false;
        _state.show_window_event_units = false;
        _state.show_window_event_text = false;
        _state.show_window_map_lights = true;
        _state.show_window_map_records = true;
    }

    if (scene->mode == MODE_EVENT) {
        if (igButton("Prev Event (j)")) {
            scene_prev();
        }
        igSameLine();
        if (igButton("Next Event (k)")) {
            scene_next();
        }
        igNewLine();
        if (igButton("Play Event")) {
            vm_execute_event(&scene->event);
        }
        igText("Event: %d - %s", desc.event_id, desc.name);
    } else {
        if (igButton("Prev Map (j)")) {
            scene_prev();
        }
        igSameLine();
        if (igButton("Next Map (k)")) {
            scene_next();
        }
    }

    igText("Map: %d - %s", scene->current_map, map_list[scene->current_map].name);
    igNewLine();

    igCheckbox("Enable Dithering", gfx_get_dither());

    if (igCollapsingHeader("Model", ImGuiTreeNodeFlags_DefaultOpen)) {
        transform3d_t* transform = gfx_model_get_transform();
        igSliderFloat3("Model", (float*)&transform->translation.raw, -1000.0f, 1000.0f);
    }

    if (igCollapsingHeader("Camera", ImGuiTreeNodeFlags_DefaultOpen)) {
        igNewLine();
        if (igRadioButton("Orthographic", !cam->use_perspective)) {
            cam->use_perspective = false;
        }
        igSameLine();
        if (igRadioButton("Perspective", cam->use_perspective)) {
            cam->use_perspective = true;
        }
        igSeparator();
        igSliderFloat3("Position", (float*)&cam->position.raw, -1000.0f, 1000.0f);
        igSeparator();
        igSliderFloat("Zoom", &cam->zoom, 0, 2.0f);
        igSliderFloat("Yaw", &cam->yaw_rad, -3.0f, 3.0f);
        igSameLine();
        igText("%0.2f°", glm_deg(cam->yaw_rad));
        igSliderFloat("Pitch", &cam->pitch_rad, -3.0f, 3.0f);
        igSameLine();
        igText("%0.2f°", glm_deg(cam->pitch_rad));
    }

    igNewLine();
    if (igCollapsingHeader("Memory", ImGuiTreeNodeFlags_DefaultOpen)) {
        igText("Current Usage: %0.2fMB", BYTES_TO_MB(memory_state.usage_current));
        igText("Total Usage: %0.2fMB", BYTES_TO_MB(memory_state.usage_total));
        igText("Peak Usage: %0.2fMB", BYTES_TO_MB(memory_state.usage_peak));
        igSeparator();
        igText("Current Allocations: %zu", memory_state.allocations_current);
        igText("Total Allocations: %zu", memory_state.allocations_total);
    }
    igNewLine();
    if (igCollapsingHeader("Filesystem Cache", ImGuiTreeNodeFlags_DefaultOpen)) {
        igText("Cached Size: %0.2fMB", BYTES_TO_MB(filesystem_cached_size()));
        igText("Cached Files: %zu", filesystem_cached_count());
    }
    igEnd();
}

static void _draw_window_map_lights(void) {
    igBegin("Lights", &_state.show_window_map_lights, 0);

    lighting_t* lighting = gfx_model_get_lighting();
    vec4s* ambient = &lighting->ambient_color;
    igText("Ambient");
    igColorEdit4("Color", (float*)ambient->raw, ImGuiColorEditFlags_Float);
    igDragFloat("Strength", &lighting->ambient_strength);
    igSeparator();

    for (int i = 0; i < 3; i++) {
        light_t* l = &lighting->lights[i];
        if (!l->valid) {
            continue;
        }
        igPushIDInt(i);
        igText("Light %d", i);
        igColorEdit4("Color", (float*)&l->color, ImGuiColorEditFlags_Float);
        igDragFloat3("Direction", (float*)&l->direction.raw);
        igSeparator();
        igPopID();
    }

    igEnd();
}

void _row_instr_fn_camera(instruction_t* instr) {
    igTableSetColumnIndex(1);

    param_t* p = instr->params;
    f32 x = parse_coord(p[0].value.i16);
    f32 y = parse_coord(p[1].value.i16);
    f32 z = parse_coord(p[2].value.i16);
    f32 pitch = parse_rad(p[3].value.i16);
    f32 maprot = parse_rad(p[4].value.i16);
    f32 yaw = parse_rad(p[5].value.i16);
    f32 zoom = parse_zoom(p[6].value.i16);
    int duration = p[7].value.i16;

    if (igButton("MoveTo")) {
        vec3s pos = { { x, y, z } };
        camera_set_freefly(pos, yaw, pitch, zoom);
        gfx_model_set_y_rotation(maprot);
    }

    igTableSetColumnIndex(2);
    igText("X: %0.2f", x);
    igTableSetColumnIndex(3);
    igText("Y: %0.2f", y);
    igTableSetColumnIndex(4);
    igText("Z: %0.2f", z);
    igTableSetColumnIndex(5);
    igText("Pitch %0.2f°", glm_deg(pitch));
    igTableSetColumnIndex(6);
    igText("Yaw: %0.2f°", glm_deg(yaw));
    igTableSetColumnIndex(7);
    igText("MapRot: %0.2f°", glm_deg(maprot));
    igTableSetColumnIndex(8);
    igText("Zoom: %0.2f", zoom);
    igTableSetColumnIndex(9);
    igText("Time: %d", duration);
}

static void _draw_window_event_instructions(void) {
    scene_t* scene = scene_get_internals();
    igBegin("Event Instructions", &_state.show_window_event_instructions, 0);
    if (igBeginTable("", 13, ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_BordersOuterV | ImGuiTableFlags_RowBg)) {
        igTableSetupColumnEx("Type", ImGuiTableColumnFlags_WidthFixed, 200, 0);
        igTableSetupColumnEx("Action", ImGuiTableColumnFlags_WidthFixed, 50, 0);
        igTableSetupColumnEx("1", ImGuiTableColumnFlags_WidthStretch, 0.0f, 0);
        igTableSetupColumnEx("2", ImGuiTableColumnFlags_WidthStretch, 0.0f, 0);
        igTableSetupColumnEx("3", ImGuiTableColumnFlags_WidthStretch, 0.0f, 0);
        igTableSetupColumnEx("4", ImGuiTableColumnFlags_WidthStretch, 0.0f, 0);
        igTableSetupColumnEx("5", ImGuiTableColumnFlags_WidthStretch, 0.0f, 0);
        igTableSetupColumnEx("6", ImGuiTableColumnFlags_WidthStretch, 0.0f, 0);
        igTableSetupColumnEx("7", ImGuiTableColumnFlags_WidthStretch, 0.0f, 0);
        igTableSetupColumnEx("8", ImGuiTableColumnFlags_WidthFixed, 60.0f, 0);
        igTableSetupColumnEx("9", ImGuiTableColumnFlags_WidthFixed, 60.0f, 0);
        igTableSetupColumnEx("10", ImGuiTableColumnFlags_WidthFixed, 60.0f, 0);
        igTableSetupColumnEx("Extra", ImGuiTableColumnFlags_WidthFixed, 40.0f, 0);

        igTableHeadersRow();

        for (usize i = 0; i < scene->event.instruction_count; i++) {
            instruction_t* instr = &scene->event.instructions[i];
            igTableNextRow();

            // Highlight the current instruction
            if ((i + 1) == (usize)vm_get_current_instruction()) {
                igTableSetBgColor(ImGuiTableBgTarget_RowBg0, IM_COL32(70, 130, 180, 255), -1);
            }

            igTableSetColumnIndex(0);
            igText("0x%02X - %s", instr->opcode, opcode_desc_list[instr->opcode].name);

            if (instr->opcode == OPCODE_ID_CAMERA) {
                igPushIDInt(i);
                _row_instr_fn_camera(instr);
                igPopID();
                continue;
            }

            igTableSetColumnIndex(1);
            igText("");

            static bool set = false;
            for (int j = 0; j < instr->param_count; j++) {
                // This is ugly cause there is only opcode (0x73) that has more
                // than 10 params. Its rarely used and we don't care about it.
                // Instead of messing up the UI for an exception, just shove
                // them all in a final column.
                if (j <= 9) {
                    igTableSetColumnIndex(j + 2);
                } else {
                    if (!set) {
                        igTableSetColumnIndex(12);
                    }
                    set = true;
                }

                if (instr->params[j].type == PARAM_TYPE_U8) {
                    igText("0x%02X", instr->params[j].value.u8);
                } else if (instr->params[j].type == PARAM_TYPE_U16) {
                    igText("0x%04X", instr->params[j].value.u16);
                }
            }
            set = false;
        }
        igEndTable();
    }
    igEnd();
}

static void _draw_window_event_entd(void) {
    scene_t* scene = scene_get_internals();
    igBegin("Event Units", &_state.show_window_event_units, 0);
    if (igBeginTable("", 19, ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_BordersOuterV | ImGuiTableFlags_RowBg)) {
        igTableSetupColumnEx("ID", ImGuiTableColumnFlags_WidthFixed, 25, 0);
        igTableSetupColumnEx("Sprite", ImGuiTableColumnFlags_WidthFixed, 50, 0);
        igTableSetupColumnEx("Palette", ImGuiTableColumnFlags_WidthFixed, 50, 0);
        igTableSetupColumnEx("Name", ImGuiTableColumnFlags_WidthFixed, 80, 0);
        igTableSetupColumnEx("Level", ImGuiTableColumnFlags_WidthFixed, 35, 0);
        igTableSetupColumnEx("Brave/Faith", ImGuiTableColumnFlags_WidthFixed, 75, 0);
        igTableSetupColumnEx("Experience", ImGuiTableColumnFlags_WidthStretch, 0, 0);
        igTableSetupColumnEx("Pos", ImGuiTableColumnFlags_WidthFixed, 50, 0);
        igTableSetupColumnEx("Birthday", ImGuiTableColumnFlags_WidthFixed, 80, 0);
        igTableSetupColumnEx("Job", ImGuiTableColumnFlags_WidthStretch, 0, 0);
        igTableSetupColumnEx("Job Lvl", ImGuiTableColumnFlags_WidthFixed, 50, 0);
        igTableSetupColumnEx("Job Unlock", ImGuiTableColumnFlags_WidthFixed, 50, 0);
        igTableSetupColumnEx("Secondary Skill", ImGuiTableColumnFlags_WidthStretch, 0, 0);
        igTableSetupColumnEx("Abilities", ImGuiTableColumnFlags_WidthStretch, 0, 0);
        igTableSetupColumnEx("Gear", ImGuiTableColumnFlags_WidthStretch, 0, 0);
        igTableSetupColumnEx("Direction", ImGuiTableColumnFlags_WidthStretch, 0, 0);
        igTableSetupColumnEx("TargetID", ImGuiTableColumnFlags_WidthStretch, 0, 0);
        igTableSetupColumnEx("Flags", ImGuiTableColumnFlags_WidthFixed, 200, 0);

        igTableHeadersRow();

        char buf[64];

        for (usize i = 0; i < scene->units.count; i++) {
            unit_t unit = scene->units.units[i];
            igTableNextRow();
            igTableNextColumn();
            igText("%d", unit.unit_id);
            igTableNextColumn();
            igText("%d", unit.sprite_set);
            igTableNextColumn();
            igText("%d", unit.palette);
            igTableNextColumn();
            igText("%s", unit_name_str(unit.name));
            igTableNextColumn();
            unit_level_str(unit.level, buf);
            igText("%s", buf);
            igTableNextColumn();
            unit_brave_faith_str(unit.bravery, unit.faith, buf);
            igText("%s", buf);
            igTableNextColumn();
            igText("%d", unit.experience);
            igTableNextColumn();
            igText("%d x %d", unit.pos_x, unit.pos_y);
            igTableNextColumn();
            unit_bday_str(unit.birthday, buf);
            igText("%s", buf);
            igTableNextColumn();
            igText("%s", unit_job_str(unit.job));
            igTableNextColumn();
            igText("%d", unit.job_level);
            igTableNextColumn();
            igText("%d", unit.job_unlock);
            igTableNextColumn();
            igText("%s", unit_skill_str(unit.secondary_job));
            igTableNextColumn();
            igText("Rct: %s", unit_ability_str(unit.reaction));
            igText("Sup: %s", unit_ability_str(unit.support));
            igText("Mov: %s", unit_ability_str(unit.movement));
            igTableNextColumn();
            igText("Head: %s", unit_item_str(unit.head));
            igText("Body: %s", unit_item_str(unit.body));
            igText("Accs: %s", unit_item_str(unit.accessory));
            igText("L: %s", unit_item_str(unit.left_hand));
            igText("R: %s", unit_item_str(unit.right_hand));
            igTableNextColumn();
            igText("%s", unit_direction_str(unit.direction));
            igTableNextColumn();
            igText("%d", unit.target_unit_id);
            igTableNextColumn();
            unit_flags_a_str(unit.flags_a, buf);
            igText("A: %s", buf);
            unit_flags_b_str(unit.flags_b, buf);
            igText("B: %s", buf);
            unit_flags_c_str(unit.flags_c, buf);
            igText("C: %s", buf);
        }
        igEndTable();
    }
    igEnd();
}

static void _draw_window_terrain(void) {
    scene_t* scene = scene_get_internals();
    int x_count = scene->map->primary_mesh.terrain.x_count;
    int z_count = scene->map->primary_mesh.terrain.z_count;

    igBegin("Terrain", &_state.show_window_terrain, 0);

    igText("Terrain: %d x %d = %d", x_count, z_count, x_count * z_count);

    if (igBeginTable("", 13, ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_BordersOuterV | ImGuiTableFlags_RowBg)) {
        igTableSetupColumnEx("#", ImGuiTableColumnFlags_WidthFixed, 20, 0);
        igTableSetupColumnEx("lvl", ImGuiTableColumnFlags_WidthFixed, 30, 0);
        igTableSetupColumnEx("Z", ImGuiTableColumnFlags_WidthFixed, 20, 0);
        igTableSetupColumnEx("X", ImGuiTableColumnFlags_WidthFixed, 20, 0);
        igTableSetupColumnEx("Surface", ImGuiTableColumnFlags_WidthFixed, 120, 0);
        igTableSetupColumnEx("Slope", ImGuiTableColumnFlags_WidthFixed, 80, 0);
        igTableSetupColumnEx("Slope Bot/Top", ImGuiTableColumnFlags_WidthFixed, 90, 0);
        igTableSetupColumnEx("Depth", ImGuiTableColumnFlags_WidthFixed, 50, 0);
        igTableSetupColumnEx("Shading", ImGuiTableColumnFlags_WidthFixed, 50, 0);
        igTableSetupColumnEx("AutoCam", ImGuiTableColumnFlags_WidthFixed, 200, 0);
        igTableSetupColumnEx("Pass", ImGuiTableColumnFlags_WidthFixed, 40, 0);
        igTableSetupColumnEx("Walk", ImGuiTableColumnFlags_WidthFixed, 40, 0);
        igTableSetupColumnEx("Select", ImGuiTableColumnFlags_WidthFixed, 40, 0);

        igTableHeadersRow();

        for (u8 level = 0; level < 2; level++) {
            for (int z = 0; z < z_count; z++) {
                for (int x = 0; x < x_count; x++) {
                    tile_t* tile = &scene->map->primary_mesh.terrain.tiles[level][z * x_count + x];
                    igTableNextRow();
                    int tile_number = level * (z_count * x_count) + (z * x_count + x);

                    igTableSetColumnIndex(0);
                    igText("%d", tile_number);
                    igTableSetColumnIndex(1);
                    igText("%d", level);
                    igTableSetColumnIndex(2);
                    igText("%d", z);
                    igTableSetColumnIndex(3);
                    igText("%d", x);
                    igTableSetColumnIndex(4);
                    igText("%s", terrain_surface_str(tile->surface));
                    igTableSetColumnIndex(5);
                    igText("%s", terrain_slope_str(tile->slope));
                    igTableSetColumnIndex(6);
                    if (tile->sloped_height_top == 0) {
                        igText("%d", tile->sloped_height_bottom);
                    } else {
                        igText("%d -> %d", tile->sloped_height_bottom, tile->sloped_height_bottom + tile->sloped_height_top);
                    }
                    igTableSetColumnIndex(7);
                    igText("%d", tile->depth);
                    igTableSetColumnIndex(8);
                    igText("%s", terrain_shading_str(tile->shading));
                    igTableSetColumnIndex(9);

                    char cam_dir_str[TERRAIN_STR_SIZE];
                    terrain_camdir_str(tile->auto_cam_dir, cam_dir_str);
                    igText("%s", cam_dir_str);

                    igTableSetColumnIndex(10);
                    igText("%s", tile->pass_through_only ? "true" : "false");
                    igTableSetColumnIndex(11);
                    igText("%s", !tile->cant_walk ? "false" : "true");
                    igTableSetColumnIndex(12);
                    igText("%s", !tile->cant_select ? "false" : "true");
                }
            }
        }
        igEndTable();
    }
    igEnd();
}

static void _draw(void) {
    is_hovered = false;
    ImVec2 dims = {
        .x = GFX_RENDER_WIDTH * GFX_RENDER_SCALE,
        .y = GFX_RENDER_HEIGHT * GFX_RENDER_SCALE,
    };

    // This creates a parent dockspace to attach to
    igDockSpaceOverViewport();

    igBeginMainMenuBar();
    if (igBeginMenu("File")) {
        if (igMenuItem("Show Demo Window")) {
            _state.show_window_demo = !_state.show_window_demo;
        }
        if (igMenuItem("Exit")) {
            sapp_request_quit();
        }
        igEndMenu();
    }
    if (igBeginMenu("Scene")) {
        if (igMenuItem("Show Scene")) {
            _state.show_window_scene = !_state.show_window_scene;
        }
        igEndMenu();
    }
    if (igBeginMenu("Event")) {
        if (igMenuItem("Text")) {
            _state.show_window_event_text = !_state.show_window_event_text;
        }
        if (igMenuItem("Instructions")) {
            _state.show_window_event_instructions = !_state.show_window_event_instructions;
        }
        if (igMenuItem("Units")) {
            _state.show_window_event_units = !_state.show_window_event_units;
        }
        igEndMenu();
    }
    if (igBeginMenu("Map")) {
        if (igMenuItem("Records")) {
            _state.show_window_map_records = !_state.show_window_map_records;
        }
        if (igMenuItem("Raw Records")) {
            _state.show_window_raw_records = !_state.show_window_raw_records;
        }
        if (igMenuItem("Lights")) {
            _state.show_window_map_lights = !_state.show_window_map_lights;
        }
        if (igMenuItem("Mesh")) {
            _state.show_window_mesh = !_state.show_window_mesh;
        }
        if (igMenuItem("Terrain")) {
            _state.show_window_terrain = !_state.show_window_terrain;
        }
        igEndMenu();
    }
    if (igBeginMenu("Sprites")) {
        if (igMenuItem("FONT.BIN")) {
            _state.show_sprite_window[F_EVENT__FONT_BIN] = !_state.show_sprite_window[F_EVENT__FONT_BIN];
        }

        for (usize i = 0; i < sizeof(image_desc_list) / sizeof(image_desc_t); i++) {
            image_desc_t desc = image_desc_list[i];
            if (igMenuItem(desc.name)) {
                _state.show_sprite_window[desc.entry] = !_state.show_sprite_window[desc.entry];
            }
        }
        igEndMenu();
    }
    igEndMainMenuBar();

    igBegin("Viewport", NULL, ImGuiWindowFlags_NoResize);
    if (gfx_get_color_image().id != SG_INVALID_ID) {
        igImage(simgui_imtextureid(gfx_get_color_image()), dims);
    }
    if (igIsWindowHovered(ImGuiHoveredFlags_None)) {
        is_hovered = true;
    }
    igEnd();

    if (_state.show_window_scene) {
        _draw_window_scene();
    }

    if (_state.show_window_event_instructions) {
        _draw_window_event_instructions();
    }
    if (_state.show_window_event_units) {
        _draw_window_event_entd();
    }
    if (_state.show_window_event_text) {
        _draw_window_event_text();
    }

    if (_state.show_window_raw_records) {
        _draw_window_raw_records();
    }
    if (_state.show_window_map_records) {
        _draw_window_map_records();
    }

    if (_state.show_window_map_lights) {
        _draw_window_map_lights();
    }

    if (_state.show_window_mesh) {
        _draw_window_mesh();
    }

    if (_state.show_window_terrain) {
        _draw_window_terrain();
    }

    if (_state.show_sprite_window[F_EVENT__FONT_BIN]) {
        _draw_window_sprite_font();
    }

    for (usize i = 0; i < sizeof(image_desc_list) / sizeof(image_desc_t); i++) {
        file_entry_e entry = image_desc_list[i].entry;

        if (_state.show_sprite_window[entry]) {
            image_desc_t desc = image_desc_list[i];
            _draw_window_sprite_paletted(entry, desc.width, desc.height);
        }
    }

    if (_state.show_window_demo) {
        igShowDemoWindow(&_state.show_window_demo);
    }
}

static uint32_t hash_int_rand_color(u32 v) {
    v ^= v >> 13;
    v *= 0x85ebca6b;
    v ^= v >> 16;
    return v;
}

static uint32_t hash_map_state_rand_color(map_state_t state) {
    // Simple mixing hash — works well enough for small enums
    uint32_t x = (state.time << 16) | (state.weather << 8) | (state.layout);
    x ^= x >> 13;
    x *= 0x85ebca6b;
    x ^= x >> 16;
    return x;
}
