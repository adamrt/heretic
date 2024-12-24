#include "camera.h"
#include "cglm/util.h"
#include "cimgui.h"
#include "event.h"
#include "font.h"
#include "map.h"
#include "map_record.h"
#include "memory.h"
#include "message.h"
#include "opcode.h"
#include "parse.h"
#include "scene.h"
#include "sokol_app.h"
#include "sokol_gfx.h"
#include "sokol_glue.h"
#include "sokol_imgui.h"
#include "sokol_log.h"

#include "gfx.h"
#include "gui.h"
#include "sprite.h"
#include "util.h"
#include "vm.h"
#include <string.h>

static void _draw(void);

static struct {
    bool show_window_demo;
    bool show_window_frame_bin;
    bool show_window_font_bin;

    bool show_window_scene;

    bool show_window_map_lights;
    bool show_window_map_texture;
    bool show_window_map_records;

    bool show_window_scenario;
    bool show_window_scenario_text;
    bool show_window_scenario_instructions;

    bool show_texture_resources;
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

    _state.show_window_map_lights = false;
    _state.show_window_map_texture = false;
    _state.show_window_map_records = true;

    _state.show_window_scenario = true;
    _state.show_window_scenario_text = true;
    _state.show_window_scenario_instructions = true;

    _state.show_window_frame_bin = false;
    _state.show_window_font_bin = false;

    _state.show_texture_resources = false;

    _state.show_window_demo = false;
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

static void _draw_map_records(void) {
    scene_t* scene = scene_get_internals();
    igBegin("Map Records", &_state.show_window_map_records, 0);
    igCheckbox("Show Texture Resources", &_state.show_texture_resources);

    if (igBeginTable("", 8, ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_BordersOuterV | ImGuiTableFlags_RowBg)) {
        igTableSetupColumnEx("Action", ImGuiTableColumnFlags_WidthFixed, 50.0f, 0);
        igTableSetupColumnEx("Layout", ImGuiTableColumnFlags_WidthFixed, 25.0f, 0);
        igTableSetupColumnEx("Time", ImGuiTableColumnFlags_WidthStretch, 50.0f, 0);
        igTableSetupColumnEx("Weather", ImGuiTableColumnFlags_WidthStretch, 75.0f, 0);
        igTableSetupColumnEx("Type", ImGuiTableColumnFlags_WidthStretch, 75.0f, 0);
        igTableSetupColumnEx("Verts", ImGuiTableColumnFlags_WidthStretch, 50.0f, 0);
        igTableSetupColumnEx("Lights", ImGuiTableColumnFlags_WidthStretch, 50.0f, 0);
        igTableSetupColumnEx("Pallete", ImGuiTableColumnFlags_WidthStretch, 50.0f, 0);
        igTableHeadersRow();

        for (int i = 0; i < scene->map->map_data->record_count; i++) {
            map_record_t r = scene->map->map_data->records[i];
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
        }
        igEndTable();
    }
    igEnd();
}

static void _draw_map_texture(void) {
    scene_t* scene = scene_get_internals();
    igBegin("Map Texture", &_state.show_window_map_texture, 0);
    ImVec2 dims = { 256 * 2, 1024 * 2 };
    if (scene->models[0].palette.id != SG_INVALID_ID) {
        igImage(simgui_imtextureid(scene->models[0].grayscale), dims);
    }
    igEnd();
}

static void _draw_game_frame_image(void) {
    igBegin("FRAME.BIN", &_state.show_window_frame_bin, 0);
    ImVec2 dims = { 256 * 2, 288 * 2 };
    // We have an array of item labels
    static int current_item = 0;
    char buf[32];
    snprintf(buf, sizeof(buf), "Palette %d", current_item);

    // The preview label is items[current_item], which is shown before opening
    if (igBeginCombo("Palette##combo", buf, 0)) {
        for (int i = 0; i < 22; i++) {
            bool is_selected = (current_item == i);

            snprintf(buf, sizeof(buf), "Palette %d", i);
            if (igSelectable(buf)) {
                current_item = i;
                sprite_set_frame_palette(i);
            }

            if (is_selected)
                igSetItemDefaultFocus();
        }
        igEndCombo();
    }

    igSeparator();

    igImage(simgui_imtextureid(sprite_get_frame_image()), dims);
    igEnd();
}

static void _draw_game_frame_palette_image(void) {
    igBegin("FRAME.BIN Palette", &_state.show_window_frame_bin, 0);
    ImVec2 dims = { 256 * 2, 16 * 22 * 2 };
    igImage(simgui_imtextureid(sprite_get_frame_palette_image()), dims);
    igEnd();
}

static void _draw_game_font_image(void) {
    igBegin("FONT.BIN", &_state.show_window_font_bin, 0);
    ImVec2 dims = { FONT_ATLAS_WIDTH, FONT_ATLAS_HEIGHT };
    igImage(simgui_imtextureid(font_get_atlas_image()), dims);
    igEnd();
}

static void _draw_scenario_text(void) {
    scene_t* scene = scene_get_internals();
    igBegin("Scenario Text", &_state.show_window_scenario_text, 0);
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

static void _draw_scene(void) {
    camera_t* cam = camera_get_internals();
    scene_t* scene = scene_get_internals();
    igBegin("Scene", &_state.show_window_scene, 0);
    event_desc_t desc = event_get_desc_by_scenario_id(scene->current_scenario_id);

    if (igRadioButton("Events", scene->mode == MODE_SCENARIO)) {
        scene->mode = MODE_SCENARIO;
    }
    igSameLine();
    if (igRadioButton("Maps", scene->mode == MODE_MAP)) {
        scene->mode = MODE_MAP;
    }
    igText("Event: %d - %s", desc.event_id, desc.name);
    igText("Map: %d - %s", scene->current_map, map_list[scene->current_map].name);
    igNewLine();

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
        igSliderFloat("Frustum", &cam->frustum, .0f, 512.0f);
        igSliderFloat("Zoom", &cam->zoom, 0, 2.0f);
        igSliderFloat("Yaw", &cam->yaw_rad, -3.0f, 3.0f);
        igSliderFloat("Pitch", &cam->pitch_rad, -3.0f, 3.0f);
    }

    igNewLine();
    if (igCollapsingHeader("Stats", ImGuiTreeNodeFlags_DefaultOpen)) {
        igNewLine();
        igText("Memory Usage");
        igText("Current Allocations: %zu", memory_state.allocations_current);
        igText("Total Allocations: %zu", memory_state.allocations_total);
        igText("Peak Usage: %zu (%0.2fMB)", memory_state.usage_peak, BYTES_TO_MB(memory_state.usage_peak));
        igText("Total Usage: %zu (%0.2fMB)", memory_state.usage_total, BYTES_TO_MB(memory_state.usage_total));
        igText("Current Usage: %zu (%0.2fMB)", memory_state.usage_current, BYTES_TO_MB(memory_state.usage_current));
        igSeparator();
    }
    igEnd();
}

static void _draw_map_lights(void) {
    scene_t* scene = scene_get_internals();
    igBegin("Map Lights", &_state.show_window_map_lights, 0);

    vec4s* ambient = &scene->map->mesh.lighting.ambient_color;
    igText("Ambient");
    igColorEdit4("Color", (float*)ambient->raw, ImGuiColorEditFlags_Float);
    igDragFloat("Strength", &scene->map->mesh.lighting.ambient_strength);
    igSeparator();

    for (int i = 0; i < 3; i++) {
        light_t* l = &scene->map->mesh.lighting.lights[i];
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

void _row_instr_camera(instruction_t* instr) {
    scene_t* scene = scene_get_internals();
    igTableSetColumnIndex(1);
    if (igButton("MoveTo")) {
        vec3s pos = { { 0 } };
        param_t* p = instr->params;

        pos.x = parse_coord(p[0].value.i16);
        pos.y = -parse_coord(p[1].value.i16);
        pos.z = parse_coord(p[2].value.i16);
        f32 pitch = parse_rad(p[3].value.i16);
        f32 maprot = parse_rad(p[4].value.i16);
        f32 yaw = parse_rad(p[5].value.i16);
        f32 zoom = parse_zoom(p[6].value.i16);

        camera_set_freefly(pos, yaw, pitch, zoom);
        scene->models[0].transform.rotation.y = maprot;
    }
    igTableSetColumnIndex(2);
    igText("X");
    igText("%0.2f", parse_coord(instr->params[0].value.i16));
    igTableSetColumnIndex(3);
    igText("Y");
    igText("%0.2f", -parse_coord(instr->params[1].value.i16));
    igTableSetColumnIndex(4);
    igText("Z");
    igText("%0.2f", parse_coord(instr->params[2].value.i16));
    igTableSetColumnIndex(5);
    igText("Pitch");
    igText("%0.2f°", glm_deg(parse_rad(instr->params[3].value.i16)));
    igTableSetColumnIndex(6);
    igText("Yaw");
    igText("%0.2f°", glm_deg(parse_rad(instr->params[5].value.i16)));
    igTableSetColumnIndex(7);
    igText("MapRot");
    igText("%0.2f°", glm_deg(parse_rad(instr->params[4].value.i16)));
    igTableSetColumnIndex(8);
    igText("Zoom");
    igText("%0.2f", parse_zoom(instr->params[6].value.i16));
}

static void _draw_scenario_instructions(void) {
    scene_t* scene = scene_get_internals();
    igBegin("Scenario Instructions", &_state.show_window_scenario_instructions, 0);
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
        igTableSetupColumnEx("8", ImGuiTableColumnFlags_WidthFixed, 40.0f, 0);
        igTableSetupColumnEx("9", ImGuiTableColumnFlags_WidthFixed, 40.0f, 0);
        igTableSetupColumnEx("10", ImGuiTableColumnFlags_WidthFixed, 40.0f, 0);
        igTableSetupColumnEx("Extra", ImGuiTableColumnFlags_WidthFixed, 40.0f, 0);

        igTableHeadersRow();

        for (usize i = 0; i < scene->event.instruction_count; i++) {
            instruction_t* instr = &scene->event.instructions[i];
            igTableNextRow();

            // Highlight the current instruction
            if (i == (usize)vm_get_current_instruction()) {
                igTableSetBgColor(ImGuiTableBgTarget_RowBg0, IM_COL32(70, 130, 180, 255), -1);
            }

            igTableSetColumnIndex(0);
            igText("0x%02X - %s", instr->opcode, opcode_desc_list[instr->opcode].name);

            if (instr->opcode == OPCODE_ID_CAMERA) {
                igPushIDInt(i);
                _row_instr_camera(instr);
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

static void _draw(void) {
    is_hovered = false;
    ImVec2 dims = { GFX_RENDER_WIDTH + 10, GFX_RENDER_HEIGHT + 10 };

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
    if (igBeginMenu("Scenario")) {
        if (igMenuItem("Data")) {
            _state.show_window_scenario = !_state.show_window_scenario;
        }
        if (igMenuItem("Text")) {
            _state.show_window_scenario_text = !_state.show_window_scenario_text;
        }
        if (igMenuItem("Instructions")) {
            _state.show_window_scenario_instructions = !_state.show_window_scenario_instructions;
        }
        igEndMenu();
    }
    if (igBeginMenu("Map")) {
        if (igMenuItem("Records")) {
            _state.show_window_map_records = !_state.show_window_map_records;
        }
        if (igMenuItem("Lights")) {
            _state.show_window_map_lights = !_state.show_window_map_lights;
        }
        if (igMenuItem("Texture")) {
            _state.show_window_map_texture = !_state.show_window_map_texture;
        }
        igEndMenu();
    }
    if (igBeginMenu("Sprites")) {
        if (igMenuItem("FRAME.BIN")) {
            _state.show_window_frame_bin = !_state.show_window_frame_bin;
        }
        if (igMenuItem("FONT.BIN")) {
            _state.show_window_font_bin = !_state.show_window_font_bin;
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

    igBegin("Scenario", &_state.show_window_scenario, 0);
    if (igButton("Play Scenario")) {
        scene_t* scene = scene_get_internals();
        vm_execute_event(&scene->event);
    }
    igEnd();

    if (_state.show_window_scene) {
        _draw_scene();
    }

    if (_state.show_window_scenario_instructions) {
        _draw_scenario_instructions();
    }
    if (_state.show_window_scenario_text) {
        _draw_scenario_text();
    }

    if (_state.show_window_font_bin) {
        _draw_game_font_image();
    }

    if (_state.show_window_frame_bin) {
        _draw_game_frame_image();
        _draw_game_frame_palette_image();
    }

    if (_state.show_window_map_records) {
        _draw_map_records();
    }

    if (_state.show_window_map_lights) {
        _draw_map_lights();
    }

    if (_state.show_window_map_texture) {
        _draw_map_texture();
    }

    if (_state.show_window_demo) {
        igShowDemoWindow(&_state.show_window_demo);
    }
}
