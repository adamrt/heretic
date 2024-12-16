#include <stdio.h>
#include <string.h>

#include "cglm/types-struct.h"
#include "cglm/util.h"
#include "sokol_app.h"
#include "sokol_gfx.h"
#include "sokol_log.h"

// If these are changed, update the libs.c file as well.
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_DEFAULT_FONT
#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_STANDARD_BOOL
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#include "nuklear.h"
#include "util/sokol_nuklear.h"

#include "camera.h"
#include "event.h"
#include "gfx.h"
#include "gui.h"
#include "io.h"
#include "memory.h"
#include "scenario.h"
#include "scene.h"
#include "time.h"
#include "util.h"

static struct {
    bool show_scenarios;
    bool show_messages;
    bool show_instructions;
    int scale_divisor;

    f32 xz_scale;
    f32 y_scale;
} _state;

static void _draw(void);
static void _draw_mode_selector(struct nk_context* ctx);

static void _draw_window_scenarios(struct nk_context* ctx);
static void _draw_window_messages(struct nk_context* ctx);
static void _draw_window_instructions(struct nk_context* ctx);

static void _draw_section_scenario(struct nk_context* ctx);
static void _draw_section_map(struct nk_context* ctx);
static void _draw_section_lights(struct nk_context* ctx);
static void _draw_section_camera(struct nk_context* ctx);
static void _draw_section_game_camera(struct nk_context* ctx);
static void _draw_section_orbit_camera(struct nk_context* ctx);
static void _draw_section_memory(struct nk_context* ctx);
static void _draw_section_misc(struct nk_context* ctx);

static void _draw_dropdown_map(struct nk_context* ctx);
static void _draw_dropdown_scenario(struct nk_context* ctx);

void gui_init(void) {
    snk_setup(&(snk_desc_t) {
        .dpi_scale = sapp_dpi_scale(),
        .logger.func = slog_func,
    });

    _state.show_instructions = false;
    _state.show_scenarios = false;
    _state.show_messages = false;
    _state.xz_scale = 4.0f;
    _state.y_scale = 2.0f;
}

void gui_update(void) {
    _draw();
    snk_render(sapp_width(), sapp_height());
}

bool gui_input(const sapp_event* event) {
    return snk_handle_event(event);
}

void gui_shutdown(void) {
    snk_shutdown();
}

// draw is the main function that draws the GUI.
static void _draw(void) {
    struct nk_context* ctx = snk_new_frame();

    nk_flags window_flags = NK_WINDOW_BORDER | NK_WINDOW_SCALABLE | NK_WINDOW_MOVABLE | NK_WINDOW_MINIMIZABLE;
    if (nk_begin(ctx, "Heretic", nk_rect(10, 25, 400, 600), window_flags)) {
        _draw_mode_selector(ctx);

        scene_t* scene = scene_get_internals();
        if (scene->mode == MODE_SCENARIO) {
            _draw_section_scenario(ctx);
        } else if (scene->mode == MODE_MAP) {
            _draw_section_map(ctx);
        }

        _draw_section_lights(ctx);
        _draw_section_camera(ctx);
        _draw_section_memory(ctx);
        _draw_section_misc(ctx);
    }
    nk_end(ctx);

    if (_state.show_instructions) {
        _draw_window_instructions(ctx);
    }

    if (_state.show_scenarios) {
        _draw_window_scenarios(ctx);
    }

    if (_state.show_messages) {
        _draw_window_messages(ctx);
    }

    return;
}

static void _draw_section_camera(struct nk_context* ctx) {
    if (nk_tree_push(ctx, NK_TREE_TAB, "Camera", NK_MINIMIZED)) {
        camera_t* cam = camera_get_internals();

        nk_layout_row_dynamic(ctx, 25, 1);
        nk_checkbox_label(ctx, "Perspective", &cam->use_perspective);

        nk_layout_row_dynamic(ctx, 25, 2);
        nk_labelf(ctx, NK_TEXT_LEFT, "X: %.2f", cam->position.x);
        nk_slider_float(ctx, -4096.0f, &cam->position.x, 4096.0f, 0.1f);
        nk_labelf(ctx, NK_TEXT_LEFT, "Y: %.2f", cam->position.y);
        nk_slider_float(ctx, -4096.0f, &cam->position.y, 4096.0f, 0.1f);
        nk_labelf(ctx, NK_TEXT_LEFT, "Z: %.2f", cam->position.z);
        nk_slider_float(ctx, -4096.0f, &cam->position.z, 4096.0f, 0.1f);

        nk_labelf(ctx, NK_TEXT_LEFT, "Yaw: %.2f", cam->yaw_rad);
        nk_slider_float(ctx, -2.0f, &cam->yaw_rad, 2.0f, 0.1f);
        nk_labelf(ctx, NK_TEXT_LEFT, "Pitch: %.2f", cam->pitch_rad);
        nk_slider_float(ctx, -2.0f, &cam->pitch_rad, 2.0f, 0.1f);

        nk_tree_pop(ctx);
    }
}

static void _draw_mode_selector(struct nk_context* ctx) {
    scene_t* scene = scene_get_internals();
    nk_layout_row_static(ctx, 30, 100, 2);

    scene->mode = nk_option_label(ctx, "Maps", scene->mode == MODE_MAP) ? MODE_MAP : scene->mode;
    scene->mode = nk_option_label(ctx, "Scenarios", scene->mode == MODE_SCENARIO) ? MODE_SCENARIO : scene->mode;
}

static void _draw_section_scenario(struct nk_context* ctx) {

    if (nk_tree_push(ctx, NK_TREE_TAB, "Scenario", NK_MAXIMIZED)) {
        _draw_dropdown_scenario(ctx);

        scene_t* scene = scene_get_internals();
        scenario_t scenario = io_get_scenario(scene->current_scenario);
        map_desc_t map = map_list[scenario.map_id];

        nk_layout_row_dynamic(ctx, 25, 1);
        nk_labelf(ctx, NK_TEXT_LEFT, "Map %d: %s", map.id, map.name);
        nk_labelf(ctx, NK_TEXT_LEFT, "Time: %s, Weather: %s, Layout: %d", time_str(scenario.time), weather_str(scenario.weather), 0);

        nk_checkbox_label(ctx, "Show Event Instructions", &_state.show_instructions);
        nk_checkbox_label(ctx, "Show Event Messages", &_state.show_messages);

        nk_spacer(ctx);
        nk_tree_pop(ctx);
    }
}

static void _draw_section_map(struct nk_context* ctx) {
    if (nk_tree_push(ctx, NK_TREE_TAB, "Map", NK_MAXIMIZED)) {
        _draw_dropdown_map(ctx);
        nk_spacer(ctx);

        nk_tree_pop(ctx);
    }
}

static void _draw_section_lights(struct nk_context* ctx) {
    scene_t* scene = scene_get_internals();
    if (nk_tree_push(ctx, NK_TREE_TAB, "Lighting", NK_MINIMIZED)) {
        nk_layout_row_dynamic(ctx, 25, 3);

        nk_label(ctx, "Ambient", NK_TEXT_LEFT);

        nk_slider_float(ctx, 0, &scene->map->mesh.lighting.ambient_strength, 3.0f, 0.1f);

        vec4s* ambient_color = &scene->map->mesh.lighting.ambient_color;
        struct nk_colorf ambient_color_nk = { ambient_color->r, ambient_color->g, ambient_color->b, ambient_color->a };
        if (nk_combo_begin_color(ctx, nk_rgba_f(ambient_color->r, ambient_color->g, ambient_color->b, ambient_color->a), nk_vec2(200, 400))) {
            ambient_color_nk = nk_color_picker(ctx, ambient_color_nk, NK_RGBA);

            nk_option_label(ctx, "RGBA", true);
            ambient_color_nk.r = nk_propertyf(ctx, "#R:", 0, ambient_color_nk.r, 1.0f, 0.01f, 0.005f);
            ambient_color_nk.g = nk_propertyf(ctx, "#G:", 0, ambient_color_nk.g, 1.0f, 0.01f, 0.005f);
            ambient_color_nk.b = nk_propertyf(ctx, "#B:", 0, ambient_color_nk.b, 1.0f, 0.01f, 0.005f);
            ambient_color_nk.a = nk_propertyf(ctx, "#A:", 0, ambient_color_nk.a, 1.0f, 0.01f, 0.005f);

            *ambient_color = (vec4s) { { ambient_color_nk.r, ambient_color_nk.g, ambient_color_nk.b, ambient_color_nk.a } };
            nk_combo_end(ctx);
        }

        nk_spacer(ctx);

        for (int i = 0; i < LIGHTING_MAX_LIGHTS; i++) {
            nk_layout_row_dynamic(ctx, 25, 3);
            light_t* light = &scene->map->mesh.lighting.lights[i];
            if (!light->valid) {
                continue;
            }

            nk_labelf(ctx, NK_TEXT_LEFT, "Light %d", i + 1);

            char posbuffer[64];
            snprintf(posbuffer, 64, "%.2f, %.2f, %.2f", light->direction.x, light->direction.y, light->direction.z);
            if (nk_combo_begin_label(ctx, posbuffer, nk_vec2(200, 200))) {
                nk_layout_row_dynamic(ctx, 25, 1);
                nk_property_float(ctx, "#X:", -30.0f, &light->direction.x, 30.0f, 1, 0.5f);
                nk_property_float(ctx, "#Y:", -30.0f, &light->direction.y, 30.0f, 1, 0.5f);
                nk_property_float(ctx, "#Z:", -30.0f, &light->direction.z, 30.0f, 1, 0.5f);
                nk_combo_end(ctx);
            }

            struct nk_colorf light_color_nk = { light->color.r, light->color.g, light->color.b, light->color.a };
            if (nk_combo_begin_color(ctx, nk_rgba_f(light->color.r, light->color.g, light->color.b, light->color.a), nk_vec2(200, 400))) {
                nk_layout_row_dynamic(ctx, 120, 1);

                light_color_nk = nk_color_picker(ctx, light_color_nk, NK_RGBA);

                nk_layout_row_dynamic(ctx, 25, 2);
                nk_layout_row_dynamic(ctx, 25, 2);
                nk_option_label(ctx, "RGBA", true);
                nk_layout_row_dynamic(ctx, 25, 1);

                light_color_nk.r = nk_propertyf(ctx, "#R:", 0, light_color_nk.r, 1.0f, 0.01f, 0.005f);
                light_color_nk.g = nk_propertyf(ctx, "#G:", 0, light_color_nk.g, 1.0f, 0.01f, 0.005f);
                light_color_nk.b = nk_propertyf(ctx, "#B:", 0, light_color_nk.b, 1.0f, 0.01f, 0.005f);
                light_color_nk.a = nk_propertyf(ctx, "#A:", 0, light_color_nk.a, 1.0f, 0.01f, 0.005f);

                light->color = (vec4s) { { light_color_nk.r, light_color_nk.g, light_color_nk.b, light_color_nk.a } };
                nk_combo_end(ctx);
            }
        }
        nk_tree_pop(ctx);
    }
}

static void _draw_section_memory(struct nk_context* ctx) {
    if (nk_tree_push(ctx, NK_TREE_TAB, "Memory", NK_MINIMIZED)) {
        nk_layout_row_dynamic(ctx, 20, 1);
        nk_labelf(ctx, NK_TEXT_LEFT, "Current Allocations: %zu", memory_state.allocations_current);
        nk_labelf(ctx, NK_TEXT_LEFT, "Total Allocations: %zu", memory_state.allocations_total);
        nk_labelf(ctx, NK_TEXT_LEFT, "Peak Usage: %zu (%0.2fMB)", memory_state.usage_peak, BYTES_TO_MB(memory_state.usage_peak));
        nk_labelf(ctx, NK_TEXT_LEFT, "Total Usage: %zu (%0.2fMB)", memory_state.usage_total, BYTES_TO_MB(memory_state.usage_total));
        nk_labelf(ctx, NK_TEXT_LEFT, "Current Usage: %zu (%0.2fMB)", memory_state.usage_current, BYTES_TO_MB(memory_state.usage_current));
        nk_tree_pop(ctx);
    }
}

static void _draw_section_misc(struct nk_context* ctx) {
    scene_t* scene = scene_get_internals();
    if (nk_tree_push(ctx, NK_TREE_TAB, "Global", NK_MINIMIZED)) {
        nk_layout_row_dynamic(ctx, 25, 2);
        nk_checkbox_label(ctx, "Show Game Scenarios", &_state.show_scenarios);
        nk_checkbox_label(ctx, "Center Model", &scene->center_model);
        nk_layout_row_static(ctx, 20, 40, 6);
        nk_label(ctx, "Scale", NK_TEXT_LEFT);

        int current = gfx_get_scale_divisor();
        int new_value = current;
        new_value = nk_option_label(ctx, "1", new_value == 1) ? 1 : new_value;
        new_value = nk_option_label(ctx, "2", new_value == 2) ? 2 : new_value;
        new_value = nk_option_label(ctx, "3", new_value == 3) ? 3 : new_value;
        new_value = nk_option_label(ctx, "4", new_value == 4) ? 4 : new_value;
        new_value = nk_option_label(ctx, "8", new_value == 8) ? 8 : new_value;
        if (new_value != current) {
            gfx_set_scale_divisor(new_value);
        }

        nk_tree_pop(ctx);
    }
}

static void _draw_window_scenarios(struct nk_context* ctx) {
    if (nk_begin(ctx, "Scenarios", nk_rect(10, GFX_DISPLAY_HEIGHT - 250, 1270, 200), NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_CLOSABLE | NK_WINDOW_MINIMIZABLE)) {
        nk_layout_row_dynamic(ctx, 15, 2);
        for (int i = 0; i < SCENARIO_COUNT; i++) {
            scenario_t scenario = io_get_scenario(i);

            nk_labelf(ctx, NK_TEXT_LEFT, "ID: %d, Event ID: %d, Next: %d, Map: %s", i, scenario.event_id, scenario.next_scenario_id, scenario_name_list[scenario.event_id].name);
            nk_labelf(ctx, NK_TEXT_LEFT, "Weather: %s, Time: %s, ENTD: %d", weather_str(scenario.weather), time_str(scenario.time), scenario.entd_id);
        }
    }
    nk_end(ctx);
}

static void _draw_window_messages(struct nk_context* ctx) {
    if (nk_begin(ctx, "Messages", nk_rect(GFX_DISPLAY_WIDTH - 620, 20, 600, 960), NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_CLOSABLE | NK_WINDOW_MINIMIZABLE)) {
        nk_layout_row_dynamic(ctx, 20, 1);
        const event_t event = scene_get_event();
        const char* start = event.messages;
        const char* end = event.messages + event.messages_len;

        char delimiter = 0xFE;

        while (start < end) {
            const char* delimiter_pos = memchr(start, delimiter, end - start);
            const usize len = delimiter_pos ? (delimiter_pos - start) : (end - start);

            nk_labelf(ctx, NK_TEXT_LEFT, "%.*s", (int)len, start);

            if (delimiter_pos) {
                start = delimiter_pos + 1;
            } else {
                break; // No more delimiters
            }
        }
    }
    nk_end(ctx);
}

static void _draw_window_instructions(struct nk_context* ctx) {
    // scene_t* scene = scene_get_internals();
    camera_t* cam = camera_get_internals();
    if (nk_begin(ctx, "Instructions", nk_rect(GFX_DISPLAY_WIDTH - 1044, 20, 1024, 800), NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_CLOSABLE | NK_WINDOW_MINIMIZABLE)) {
        nk_layout_row_begin(ctx, NK_STATIC, 20, 16);
        event_t event = scene_get_event();

        nk_layout_row_push(ctx, 60);
        nk_label(ctx, "Select", NK_TEXT_RIGHT);
        nk_label(ctx, "OpCode", NK_TEXT_RIGHT);
        nk_label(ctx, "X", NK_TEXT_RIGHT);
        nk_label(ctx, "Y", NK_TEXT_RIGHT);
        nk_label(ctx, "Z", NK_TEXT_RIGHT);
        nk_label(ctx, "Pitch", NK_TEXT_RIGHT);
        nk_label(ctx, "MapRot", NK_TEXT_RIGHT);
        nk_label(ctx, "Yaw", NK_TEXT_RIGHT);
        nk_label(ctx, "Zoom", NK_TEXT_RIGHT);
        nk_label(ctx, "Timer", NK_TEXT_RIGHT);
        nk_layout_row_end(ctx);
        nk_layout_row_begin(ctx, NK_STATIC, 20, 16);
        for (int i = 0; i < event.instruction_count; i++) {
            instruction_t instruction = event.instructions[i];
            opcode_t opcode = opcode_list[instruction.code];
            if (instruction.code != 0x19)
                continue;

            nk_layout_row_push(ctx, 60);
            if (nk_button_label(ctx, "MoveTo")) {
                f32 x = (f32)((i16)instruction.parameters[0].value.u16);
                f32 y = -(f32)((i16)instruction.parameters[1].value.u16);
                f32 z = (f32)((i16)instruction.parameters[2].value.u16);

                cam->position.x = x / _state.xz_scale;
                cam->position.y = y / _state.y_scale;
                cam->position.z = z / _state.xz_scale;
                printf("xz_scale: %f, y_scale: %f\n", _state.xz_scale, _state.y_scale);

                // f32 pitch = -(f32)((i16)instruction.parameters[3].value.u16);
                // cam->pitch = (pitch / 1024.0f) * 90.0f;

                /* f32 maprot = -(f32)((i16)instruction.parameters[4].value.u16); */
                /* f32 maprot_scaled = (maprot * 360.0f) / 4096.0f; */
                /* f32 maprot_rad = glm_rad(maprot_scaled); */
                // scene->models.transform.rotation.y = maprot_rad;
            }

            if (opcode.name != NULL) {
                nk_label(ctx, opcode.name, NK_TEXT_LEFT);
            } else {
                nk_labelf(ctx, NK_TEXT_LEFT, "NO OPCODE ---- (0x%X)", instruction.code);
            }
            for (int j = 0; j < EVENT_PARAMETER_MAX; j++) {
                parameter_t param = instruction.parameters[j];
                if (param.type == PARAMETER_TYPE_NONE) {
                    nk_labelf(ctx, NK_TEXT_RIGHT, "%d", 0);
                } else if (param.type == PARAMETER_TYPE_U16) {

                    f32 value = (f32)((i16)param.value.u16);
                    if (j == 1) {
                        value = -value;
                        value = value / 2.0f;
                    }
                    if (j == 0 || j == 2)
                        value = value / 4.0f;
                    if (j == 3) {
                        value = (value / 1024.0f) * 90.0f;
                    }
                    nk_labelf(ctx, NK_TEXT_RIGHT, "%0.1f", value);
                } else {
                    f32 value = (f32)(i8)param.value.u8;
                    nk_labelf(ctx, NK_TEXT_RIGHT, "%0.1f", value);
                }
            }
            nk_layout_row_end(ctx);
        }
    }
    nk_end(ctx);
}

static void _draw_dropdown_map(struct nk_context* ctx) {
    scene_t* scene = scene_get_internals();

    map_desc_t selected_map = map_list[scene->current_map];
    map_state_t map_state = scene->map->map_state;

    char buffer[64];

    nk_layout_row_begin(ctx, NK_STATIC, 25, 2);
    nk_layout_row_push(ctx, 50);
    nk_label(ctx, "Map", NK_TEXT_LEFT);
    snprintf(buffer, 64, "%d %s", selected_map.id, map_list[selected_map.id].name);

    nk_layout_row_push(ctx, 300);
    if (nk_combo_begin_label(ctx, buffer, nk_vec2(370, 550))) {
        nk_layout_row_dynamic(ctx, 25, 1);

        for (int i = 0; i < MAP_COUNT; ++i) {
            if (!map_list[i].valid) {
                continue;
            }

            snprintf(buffer, 64, "%d %s", map_list[i].id, map_list[i].name);

            if (nk_combo_item_label(ctx, buffer, NK_TEXT_LEFT)) {
                scene->current_map = i;
                scene_load_map(scene->current_map, default_map_state);
            }
        }
        nk_combo_end(ctx);
    }
    nk_layout_row_end(ctx);

    // Find unique map_states for all records.
    // This way we only show unique map_states in the dropdown.
    // It doesn't matter if the map_state is from a different record type.

    map_record_t* records = scene->map->map_data->records;
    map_record_t unique_records[MAP_RECORD_MAX_NUM];
    int unique_record_count = 0;
    for (int i = 0; i < scene->map->map_data->record_count; i++) {
        if (map_record_state_unique(unique_records, unique_record_count, records[i])) {
            unique_records[unique_record_count++] = records[i];
        }
    }

    nk_layout_row_begin(ctx, NK_STATIC, 25, 2);
    nk_layout_row_push(ctx, 50);
    nk_label(ctx, "Env", NK_TEXT_LEFT);

    snprintf(buffer, 64, "Time: %s, Weather: %s, Layout: %d", time_str(map_state.time), weather_str(map_state.weather), map_state.layout);

    nk_layout_row_push(ctx, 300);
    if (nk_combo_begin_label(ctx, buffer, nk_vec2(370, 550))) {
        nk_layout_row_dynamic(ctx, 25, 1);

        for (int i = 0; i < unique_record_count; ++i) {
            map_record_t record = unique_records[i];

            snprintf(buffer, 64, "Time: %s, Weather: %s, Layout: %d", time_str(record.state.time), weather_str(record.state.weather), record.state.layout);

            if (nk_combo_item_label(ctx, buffer, NK_TEXT_LEFT)) {
                scene_load_map(scene->current_map, record.state);
            }
        }
        nk_combo_end(ctx);
    }

    nk_layout_row_end(ctx);
}

static void _draw_dropdown_scenario(struct nk_context* ctx) {
    scene_t* scene = scene_get_internals();
    scenario_t selected_scenario = io_get_scenario(scene->current_scenario);

    nk_layout_row_begin(ctx, NK_STATIC, 25, 2);
    nk_layout_row_push(ctx, 50);
    nk_label(ctx, "Scenaio", NK_TEXT_LEFT);

    char selected_buffer[64];
    snprintf(selected_buffer, 64, "%d - %d %s", scene->current_scenario, selected_scenario.event_id, scenario_name_list[selected_scenario.event_id].name);

    nk_layout_row_push(ctx, 300);
    if (nk_combo_begin_label(ctx, selected_buffer, nk_vec2(480, 550))) {
        nk_layout_row_dynamic(ctx, 25, 1);

        for (int i = 0; i < SCENARIO_COUNT; ++i) {
            // We only want to show scenarios that have valid events.
            // The others are for load out screens and such.
            scenario_t scenario = io_get_scenario(i);
            event_t event = io_get_event(scenario.event_id);
            if (!event.valid) {
                continue;
            }
            char item_buffer[64];
            snprintf(item_buffer, 64, "%d - %d %s", i, scenario_name_list[scenario.event_id].id, scenario_name_list[scenario.event_id].name);

            if (nk_combo_item_label(ctx, item_buffer, NK_TEXT_LEFT)) {
                scene->current_scenario = i;
                scene_load_scenario(scene->current_scenario);
            }
        }
        nk_combo_end(ctx);
    }
    nk_layout_row_end(ctx);
}
