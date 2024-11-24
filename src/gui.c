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
#include "game.h"
#include "gfx.h"
#include "gui.h"
#include "scenario.h"

static struct {
    nk_bool show_scenarios;
} state;

static void draw(void);
static void draw_window_scenarios(struct nk_context* ctx);
static void draw_section_fps(struct nk_context* ctx);
static void draw_section_scene(struct nk_context* ctx);
static void draw_section_lights(struct nk_context* ctx);
static void draw_section_camera(struct nk_context* ctx);
static void draw_section_misc(struct nk_context* ctx);
static void draw_dropdown_map(struct nk_context* ctx);
static void draw_dropdown_scenario(struct nk_context* ctx);

void gui_init(void)
{
    snk_setup(&(snk_desc_t) {
        .dpi_scale = sapp_dpi_scale(),
        .logger.func = slog_func,
    });

    state.show_scenarios = false;
}

void gui_update(void)
{
    draw();
    snk_render(sapp_width(), sapp_height());
}

bool gui_input(const sapp_event* event)
{
    return snk_handle_event(event);
}

void gui_shutdown(void)
{
    snk_shutdown();
}

// draw is the main function that draws the GUI.
static void draw(void)
{
    struct nk_context* ctx = snk_new_frame();

    nk_flags window_flags = NK_WINDOW_BORDER | NK_WINDOW_SCALABLE | NK_WINDOW_MOVABLE | NK_WINDOW_MINIMIZABLE;
    if (nk_begin(ctx, "Heretic", nk_rect(10, 25, 400, 600), window_flags)) {
        draw_section_fps(ctx);
        draw_section_scene(ctx);
        draw_section_lights(ctx);
        draw_section_camera(ctx);
        draw_section_misc(ctx);
    }
    nk_end(ctx);

    if (state.show_scenarios) {
        draw_window_scenarios(ctx);
    }

    return;
}

static void draw_section_fps(struct nk_context* ctx)
{
    nk_layout_row_dynamic(ctx, 25, 1);
    char buffer[64];
    snprintf(buffer, sizeof(buffer), "FPS: %f", g.time.fps);
    nk_label(ctx, buffer, NK_TEXT_LEFT);
}

static void draw_section_scene(struct nk_context* ctx)
{
    if (nk_tree_push(ctx, NK_TREE_TAB, "Scene", NK_MAXIMIZED)) {
        nk_layout_row_static(ctx, 30, 100, 2);

        g.mode = nk_option_label(ctx, "Maps", g.mode == MODE_MAP) ? MODE_MAP : g.mode;
        g.mode = nk_option_label(ctx, "Scenarios", g.mode == MODE_SCENARIO) ? MODE_SCENARIO : g.mode;

        if (g.mode == MODE_SCENARIO) {
            draw_dropdown_scenario(ctx);
        } else if (g.mode == MODE_MAP) {
            draw_dropdown_map(ctx);
        }
        nk_tree_pop(ctx);
    }
}

static void draw_section_camera(struct nk_context* ctx)
{
    if (nk_tree_push(ctx, NK_TREE_TAB, "Camera", NK_MINIMIZED)) {
        nk_layout_row_dynamic(ctx, 25, 1);
        nk_checkbox_label(ctx, "Perspective", &g.cam.use_perspective);

        char buffer[64];
        snprintf(buffer, sizeof(buffer), "Cardinal: %s", camera_cardinal_str());
        nk_label(ctx, buffer, NK_TEXT_LEFT);

        snprintf(buffer, sizeof(buffer), "Azimuth Degrees: %f", g.cam.azimuth);
        nk_label(ctx, buffer, NK_TEXT_LEFT);

        snprintf(buffer, sizeof(buffer), "Elevation Degrees: %f", g.cam.elevation);
        nk_label(ctx, buffer, NK_TEXT_LEFT);

        nk_layout_row_dynamic(ctx, 25, 2);

        snprintf(buffer, sizeof(buffer), "Distance: %f", g.cam.distance);
        nk_label(ctx, buffer, NK_TEXT_LEFT);
        nk_slider_float(ctx, 0.001f, &g.cam.distance, CAM_DIST_MAX, 0.1f);

        snprintf(buffer, sizeof(buffer), "Near: %f", g.cam.znear);
        nk_label(ctx, buffer, NK_TEXT_LEFT);
        nk_slider_float(ctx, 0.01f, &g.cam.znear, CAM_ZFAR_MAX, 0.1f);

        snprintf(buffer, sizeof(buffer), "Far: %f", g.cam.zfar);
        nk_label(ctx, buffer, NK_TEXT_LEFT);
        nk_slider_float(ctx, 0.01f, &g.cam.zfar, CAM_ZFAR_MAX, 0.1f);

        nk_tree_pop(ctx);
    }
}

static void draw_section_lights(struct nk_context* ctx)
{

    if (nk_tree_push(ctx, NK_TREE_TAB, "Lighting", NK_MINIMIZED)) {
        nk_layout_row_dynamic(ctx, 25, 1);

        nk_label(ctx, "Ambient Strenght and Color", NK_TEXT_LEFT);

        nk_layout_row_dynamic(ctx, 25, 2);
        nk_slider_float(ctx, 0, &g.scene.map->mesh.lighting.ambient_strength, 3.0f, 0.1f);

        vec4s* ambient_color = &g.scene.map->mesh.lighting.ambient_color;
        struct nk_colorf ambient_color_nk = { ambient_color->r, ambient_color->g, ambient_color->b, ambient_color->a };
        if (nk_combo_begin_color(ctx, nk_rgba_f(ambient_color->r, ambient_color->g, ambient_color->b, ambient_color->a), nk_vec2(200, 400))) {
            nk_layout_row_dynamic(ctx, 120, 1);

            ambient_color_nk = nk_color_picker(ctx, ambient_color_nk, NK_RGBA);

            nk_layout_row_dynamic(ctx, 25, 2);
            nk_option_label(ctx, "RGBA", true);
            nk_layout_row_dynamic(ctx, 25, 1);

            ambient_color_nk.r = nk_propertyf(ctx, "#R:", 0, ambient_color_nk.r, 1.0f, 0.01f, 0.005f);
            ambient_color_nk.g = nk_propertyf(ctx, "#G:", 0, ambient_color_nk.g, 1.0f, 0.01f, 0.005f);
            ambient_color_nk.b = nk_propertyf(ctx, "#B:", 0, ambient_color_nk.b, 1.0f, 0.01f, 0.005f);
            ambient_color_nk.a = nk_propertyf(ctx, "#A:", 0, ambient_color_nk.a, 1.0f, 0.01f, 0.005f);

            *ambient_color = (vec4s) { { ambient_color_nk.r, ambient_color_nk.g, ambient_color_nk.b, ambient_color_nk.a } };
            nk_combo_end(ctx);
        }
        for (int i = 0; i < LIGHTING_MAX_LIGHTS; i++) {
            nk_layout_row_dynamic(ctx, 25, 1);
            light_t* light = &g.scene.map->mesh.lighting.lights[i];
            if (!light->valid) {
                continue;
            }

            char buffer[64];
            snprintf(buffer, 64, "Light %d", i);
            nk_label(ctx, buffer, NK_TEXT_LEFT);

            nk_layout_row_dynamic(ctx, 25, 2);
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

static void draw_section_misc(struct nk_context* ctx)
{
    if (nk_tree_push(ctx, NK_TREE_TAB, "Misc", NK_MINIMIZED)) {
        nk_layout_row_dynamic(ctx, 25, 2);
        nk_checkbox_label(ctx, "Show Scenarios", &state.show_scenarios);
        nk_checkbox_label(ctx, "Centered", &g.scene.center_model);
        nk_layout_row_static(ctx, 20, 40, 5);
        nk_label(ctx, "Scale", NK_TEXT_LEFT);

        int prev_scale_divisor = gfx.offscreen.scale_divisor;
        gfx.offscreen.scale_divisor = nk_option_label(ctx, "1", gfx.offscreen.scale_divisor == 1) ? 1 : gfx.offscreen.scale_divisor;
        gfx.offscreen.scale_divisor = nk_option_label(ctx, "2", gfx.offscreen.scale_divisor == 2) ? 2 : gfx.offscreen.scale_divisor;
        gfx.offscreen.scale_divisor = nk_option_label(ctx, "3", gfx.offscreen.scale_divisor == 3) ? 3 : gfx.offscreen.scale_divisor;
        gfx.offscreen.scale_divisor = nk_option_label(ctx, "4", gfx.offscreen.scale_divisor == 4) ? 4 : gfx.offscreen.scale_divisor;

        if (prev_scale_divisor != gfx.offscreen.scale_divisor) {
            gfx_scale_change();
        }
        nk_tree_pop(ctx);
    }
}

static void draw_window_scenarios(struct nk_context* ctx)
{
    if (nk_begin(ctx, "Scenarios", nk_rect(10, GFX_DISPLAY_HEIGHT - 250, 1270, 200), NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_CLOSABLE | NK_WINDOW_MINIMIZABLE)) {
        nk_layout_row_dynamic(ctx, 15, 2);
        for (int i = 0; i < SCENARIO_COUNT; i++) {
            scenario_t* scenario = &g.fft.scenarios[i];

            char buffer_id[64];
            snprintf(buffer_id, 64, "Scenario %d, ID: %d, Map: %s", i, scenario->id, scenario_list[scenario->id].name);

            char weather_name[12];
            weather_str(scenario->weather, weather_name);
            char time_name[8];
            time_str(scenario->time, time_name);
            char buffer_weather[40];
            snprintf(buffer_weather, 40, "Weather: %s, Time: %s, ENTD: %d", weather_name, time_name, scenario->entd_id);

            nk_label(ctx, buffer_id, NK_TEXT_LEFT);
            nk_label(ctx, buffer_weather, NK_TEXT_LEFT);
        }
    }
    nk_end(ctx);
}

static void draw_dropdown_map(struct nk_context* ctx)
{

    nk_layout_row_dynamic(ctx, 25, 1);

    map_desc_t selected_map = map_list[g.scene.current_map];
    map_state_t map_state = g.scene.map->map_state;

    char buffer[64];

    snprintf(buffer, 64, "%d %s", selected_map.id, map_list[selected_map.id].name);
    if (nk_combo_begin_label(ctx, buffer, nk_vec2(370, 550))) {
        nk_layout_row_dynamic(ctx, 25, 1);

        for (int i = 0; i < MAP_COUNT; ++i) {
            if (!map_list[i].valid) {
                continue;
            }

            snprintf(buffer, 64, "%d %s", map_list[i].id, map_list[i].name);

            if (nk_combo_item_label(ctx, buffer, NK_TEXT_LEFT)) {
                g.scene.current_map = i;
                scene_load_map(g.scene.current_map, default_map_state);
            }
        }
        nk_combo_end(ctx);
    }

    // Find unique map_states for all records.
    // This way we only show unique map_states in the dropdown.
    // It doesn't matter if the map_state is from a different record type.

    record_t* records = g.scene.map->map_data->records;
    record_t unique_records[GNS_RECORD_MAX_NUM];
    int unique_record_count = 0;
    for (int i = 0; i < g.scene.map->map_data->record_count; i++) {
        if (record_map_state_unique(unique_records, unique_record_count, records[i])) {
            unique_records[unique_record_count++] = records[i];
        }
    }

    char time_name[8];
    char weather_name[12];

    time_str(map_state.time, time_name);
    weather_str(map_state.weather, weather_name);

    snprintf(buffer, 64, "Time: %s, Weather: %s, Layout: %d", time_name, weather_name, map_state.layout);

    if (nk_combo_begin_label(ctx, buffer, nk_vec2(370, 550))) {
        nk_layout_row_dynamic(ctx, 25, 1);

        for (int i = 0; i < unique_record_count; ++i) {
            record_t record = unique_records[i];

            weather_str(record.state.weather, weather_name);
            time_str(record.state.time, time_name);

            snprintf(buffer, 64, "Time: %s, Weather: %s, Layout: %d", time_name, weather_name, record.state.layout);

            if (nk_combo_item_label(ctx, buffer, NK_TEXT_LEFT)) {
                scene_load_map(g.scene.current_map, record.state);
            }
        }
        nk_combo_end(ctx);
    }
}

static void draw_dropdown_scenario(struct nk_context* ctx)
{
    nk_layout_row_dynamic(ctx, 25, 1);

    scenario_t selected_scenario = g.fft.scenarios[g.scene.current_scenario];

    char selected_buffer[64];
    snprintf(selected_buffer, 64, "%d %s", selected_scenario.id, map_list[selected_scenario.map_id].name);

    if (nk_combo_begin_label(ctx, selected_buffer, nk_vec2(370, 550))) {
        nk_layout_row_dynamic(ctx, 25, 1);

        for (int i = 0; i < SCENARIO_COUNT; ++i) {
            // FIXME: This should be cached as it is looping this per frame.
            scenario_t scenario = g.fft.scenarios[i];
            event_t event = g.fft.events[scenario.id];
            if (!event.valid) {
                continue;
            }
            char item_buffer[64];
            snprintf(item_buffer, 64, "%d %s", scenario_list[scenario.id].id, scenario_list[scenario.id].name);

            if (nk_combo_item_label(ctx, item_buffer, NK_TEXT_LEFT)) {
                g.scene.current_scenario = i;
                scene_load_scenario(g.scene.current_scenario);
            }
        }
        nk_combo_end(ctx);
    }

    scenario_t scenario = g.fft.scenarios[g.scene.current_scenario];
    map_desc_t map = map_list[scenario.map_id];

    char weather_name[12];
    weather_str(scenario.weather, weather_name);
    char time_name[8];
    time_str(scenario.time, time_name);

    char buffer[64];
    snprintf(buffer, sizeof(buffer), "Map %d: %s", map.id, map.name);
    nk_label(ctx, buffer, NK_TEXT_LEFT);

    snprintf(buffer, sizeof(buffer), "Time: %s, Weather: %s, Layout: %d", time_name, weather_name, 0);
    nk_label(ctx, buffer, NK_TEXT_LEFT);
}
