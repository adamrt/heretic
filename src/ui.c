#include "cimgui.h"
#include "sokol_app.h"
#include "sokol_gfx.h"
#include "sokol_glue.h"
#include "sokol_imgui.h"
#include "sokol_log.h"

#include "gui.h"

static struct {
    uint64_t last_time;
    bool show_test_window;
    bool show_another_window;
} state;

static void _draw(void);

void gui_init(void) {
    simgui_setup(&(simgui_desc_t) {
        .logger.func = slog_func,
    });
}
void gui_shutdown(void) {
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
    return simgui_handle_event(event);
}

static void _draw(void) {
    // 1. Show a simple window
    // Tip: if we don't call ImGui::Begin()/ImGui::End() the widgets appears in a window automatically called "Debug"
    static float f = 0.0f;
    igText("Hello, world!");
    igSliderFloatEx("float", &f, 0.0f, 1.0f, "%.3f", ImGuiSliderFlags_None);
    if (igButton("Test Window"))
        state.show_test_window ^= 1;
    if (igButton("Another Window"))
        state.show_another_window ^= 1;
    igText("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / igGetIO()->Framerate, igGetIO()->Framerate);

    // 2. Show another simple window, this time using an explicit Begin/End pair
    if (state.show_another_window) {
        igSetNextWindowSize((ImVec2) { 200, 100 }, ImGuiCond_FirstUseEver);
        igBegin("Another Window", &state.show_another_window, 0);
        igText("Hello");
        igEnd();
    }

    // 3. Show the ImGui test window. Most of the sample code is in ImGui::ShowDemoWindow()
    if (state.show_test_window) {
        igSetNextWindowPos((ImVec2) { 460, 20 }, ImGuiCond_FirstUseEver);
        igShowDemoWindow(0);
    }
}
