#include "vm_func.h"

#include "camera.h"
#include "parse.h"
#include "scene.h"
#include "transition.h"

void fn_10_display_message(const instruction_t* instr) {
    (void)instr->params[0].value.u8; // unused, typically 0x10
    u8 type = instr->params[1].value.u8;
    printf("Display message type: 0x%X\n", type);
}

void fn_19_camera(const instruction_t* instr) {
    camera_t* cam = camera_get_internals();
    scene_t* scene = scene_get_internals();

    f32 x = parse_coord(instr->params[0].value.i16);
    f32 y = -parse_coord(instr->params[1].value.i16);
    f32 z = parse_coord(instr->params[2].value.i16);
    f32 pitch_rad = parse_rad(instr->params[3].value.i16);
    f32 maprot_rad = parse_rad(instr->params[4].value.i16);
    f32 yaw_rad = parse_rad(instr->params[5].value.i16);
    f32 zoom = parse_zoom(instr->params[6].value.i16);
    f32 duration = (f32)instr->params[7].value.i16;

    transition_add(&cam->position.x, cam->position.x, x, duration);
    transition_add(&cam->position.y, cam->position.y, y, duration);
    transition_add(&cam->position.z, cam->position.z, z, duration);
    transition_add(&cam->yaw_rad, cam->yaw_rad, yaw_rad, duration);
    transition_add(&cam->pitch_rad, cam->pitch_rad, pitch_rad, duration);
    transition_add(&cam->zoom, cam->zoom, zoom, duration);
    transition_add(&scene->models[0].transform.rotation.y, scene->models[0].transform.rotation.y, maprot_rad, duration);
}
