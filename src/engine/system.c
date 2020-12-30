//
// Created by albert on 10/5/20.
//

#include <stdlib.h>
#include <stdio.h>
#include "engine.h"

System *engine_system_create(u_int fps, u_int width, u_int height, u_int scale) {
    System *system = malloc(sizeof(System));
    system->fps = fps;
    system->timer = engine_timer_create(0);
    engine_global_clock_init();
    engine_system_resize(system, width, height, scale);
    system->input = engine_input_create();
    return system;
}

void engine_system_resize(System *system, int width, int height, int scale) {
    system->width = width;
    system->height = height;
    system->scale = scale;

    system->real_width = width * scale;
    system->real_height = height * scale;
}

void engine_system_run(System *system) {
    engine_global_clock_step();
    system->tick = engine_timer_tick(system->timer);
}

void engine_system_destroy(System *system) {
    engine_timer_destroy(system->timer);
    engine_input_destroy(system->input);
    free(system);
}

void engine_system_clear(uint32_t color) {
    tmx_col_bytes col = tmx_col_to_bytes(color);
    SDL_SetRenderDrawColor(engine_texture_global_renderer, col.r, col.g, col.b, col.a);
    SDL_RenderClear(engine_texture_global_renderer);
}

int engine_system_get_draw_pos(System *system, int pos) {
    return pos;
}

