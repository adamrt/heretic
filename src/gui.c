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

#include "bin.h"
#include "camera.h"
#include "game.h"
#include "gfx.h"
#include "gui.h"

static void gui_draw(void);

static struct {
    nk_bool show_scenarios;
    nk_bool show_lights;
    nk_bool center_model;
} state;

void gui_init(void)
{
    snk_setup(&(snk_desc_t) {
        .dpi_scale = sapp_dpi_scale(),
        .logger.func = slog_func,
    });

    state.show_scenarios = false;
    state.show_lights = false;
    state.center_model = g.scene.center_model;
}

void gui_update(void)
{
    gui_draw();
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

void render_dropdown(struct nk_context* ctx)
{
    scenario_t scenario = g.fft.scenarios[g.scene.current_scenario];

    char selected_buffer[64];
    snprintf(selected_buffer, 64, "%d %s", scenario.id, map_list[scenario.map_id].name);

    if (nk_combo_begin_label(ctx, selected_buffer, nk_vec2(370, 550))) {
        nk_layout_row_dynamic(ctx, 25, 1);

        for (int i = 0; i < SCENARIO_USABLE_COUNT; ++i) {

            scenario_t scenario = g.fft.scenarios[i];
            char item_buffer[64];
            snprintf(item_buffer, 64, "%d %s", scenario_list[scenario.id].id, scenario_list[scenario.id].name);

            if (nk_combo_item_label(ctx, item_buffer, NK_TEXT_LEFT)) {
                g.scene.current_scenario = i;
                game_load_scenario(g.scene.current_scenario);
            }
        }

        // End the combo (dropdown) box
        nk_combo_end(ctx);
    }
}
static void draw_camera(struct nk_context* ctx)
{
    nk_layout_row_dynamic(ctx, 25, 1);
    nk_checkbox_label(ctx, "Perspective", &g.cam.use_perspective);

    nk_layout_row_dynamic(ctx, 25, 2);

    char buffer[64];
    snprintf(buffer, sizeof(buffer), "Distance: %f", g.cam.distance);
    nk_label(ctx, buffer, NK_TEXT_LEFT);
    nk_slider_float(ctx, 0.001f, &g.cam.distance, CAM_MAX_DIST, 0.1f);

    snprintf(buffer, sizeof(buffer), "Near: %f", g.cam.znear);
    nk_label(ctx, buffer, NK_TEXT_LEFT);
    nk_slider_float(ctx, 0.01f, &g.cam.znear, CAM_MAX_ZFAR, 0.1f);

    snprintf(buffer, sizeof(buffer), "Far: %f", g.cam.zfar);
    nk_label(ctx, buffer, NK_TEXT_LEFT);
    nk_slider_float(ctx, 0.01f, &g.cam.zfar, CAM_MAX_ZFAR, 0.1f);
}

static void draw_lights(struct nk_context* ctx)
{

    nk_layout_row_dynamic(ctx, 25, 1);

    nk_label(ctx, "Ambient Strenght and Color", NK_TEXT_LEFT);

    nk_layout_row_dynamic(ctx, 25, 2);
    nk_slider_float(ctx, 0, &g.scene.model.mesh.lighting.ambient_strength, 3.0f, 0.1f);

    vec4s* ambient_color = &g.scene.model.mesh.lighting.ambient_color;
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
    for (int i = 0; i < MESH_MAX_LIGHTS; i++) {
        nk_layout_row_dynamic(ctx, 25, 1);
        light_t* light = &g.scene.model.mesh.lighting.lights[i];
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
}

static void draw_scenarios(struct nk_context* ctx)
{
    if (nk_begin(ctx, "Scenarios", nk_rect(10, GFX_DISPLAY_HEIGHT - 250, 1270, 200), NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_CLOSABLE | NK_WINDOW_MINIMIZABLE)) {
        nk_layout_row_dynamic(ctx, 15, 2);
        for (int i = 0; i < SCENARIO_USABLE_COUNT; i++) {
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

static void gui_draw(void)
{
    struct nk_context* ctx = snk_new_frame();

    if (state.show_scenarios) {
        draw_scenarios(ctx);
    }

    nk_flags window_flags = NK_WINDOW_BORDER | NK_WINDOW_SCALABLE | NK_WINDOW_MOVABLE | NK_WINDOW_MINIMIZABLE;

    if (nk_begin(ctx, "Heretic", nk_rect(10, 25, 400, 600), window_flags)) {
        nk_layout_row_dynamic(ctx, 25, 2);

        nk_checkbox_label(ctx, "Show Scenarios", &state.show_scenarios);
        nk_checkbox_label(ctx, "Centered", &g.scene.center_model);

        nk_layout_row_dynamic(ctx, 25, 1);

        render_dropdown(ctx);

        scenario_t scenario = g.fft.scenarios[g.scene.current_scenario];
        map_desc_t map = map_list[scenario.map_id];

        char weather_name[12];
        weather_str(scenario.weather, weather_name);
        char time_name[8];
        time_str(scenario.time, time_name);

        char buffer[64];
        snprintf(buffer, sizeof(buffer), "Map %d: %s", map.id, map.name);
        nk_label(ctx, buffer, NK_TEXT_LEFT);

        snprintf(buffer, sizeof(buffer), "Weather: %s, Time: %s", weather_name, time_name);
        nk_label(ctx, buffer, NK_TEXT_LEFT);

        if (nk_tree_push(ctx, NK_TREE_TAB, "Lighting", NK_MINIMIZED)) {
            draw_lights(ctx);
            nk_tree_pop(ctx);
        }

        if (nk_tree_push(ctx, NK_TREE_TAB, "Camera", NK_MINIMIZED)) {
            draw_camera(ctx);
            nk_tree_pop(ctx);
        }
    }
    nk_end(ctx);

    return;
}
