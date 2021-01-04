//
// Created by albert on 10/5/20.
//

#include <stdlib.h>
#include <stdio.h>
#include <allegro5/allegro.h>
#include <math.h>
#include "engine.h"

Animation *engine_animation_create(AnimationSheet *sheet, const char *name, float frame_time, size_t seq_l, const u_short *sequence, bool stop) {
    Animation *anim = malloc(sizeof(Animation));
    anim->name = name;
    anim->sheet = sheet;
    anim->timer = engine_timer_create(0);
    anim->frame_time = frame_time;
    anim->stop = stop;
    anim->flip_x = false;
    anim->flip_y = false;

    anim->sequence_l = seq_l;
    anim->sequence = malloc(sizeof (sequence[0]) * seq_l);
    for (size_t i = 0; i < seq_l; i++) {
        anim->sequence[i] = sequence[i];
    }

    anim->tile = anim->sequence[0];

    return anim;
}

void engine_animation_destroy(Animation *animation) {
    engine_timer_destroy(animation->timer);
    free(animation->sequence);
    free(animation);
}

void engine_animation_update(Animation *animation) {
    double delta = engine_timer_delta(animation->timer);
    double frame_total = delta / animation->frame_time;
    animation->loop_count = frame_total / animation->sequence_l;
    if (animation->stop && animation->loop_count > 0) {
        animation->frame = animation->sequence_l - 1;
    } else {
        animation->frame = fmod(frame_total, animation->sequence_l);
    }
    animation->tile = animation->sequence[animation->frame];
}

void engine_animation_draw(Animation *animation, int target_x, int target_y) {
    int sx, sy;

    int sheet_width = animation->sheet->image->width;

    sx = (int)animation->tile * (int)animation->sheet->width % sheet_width;
    sy = (int)animation->tile * (int)animation->sheet->width / sheet_width * (int)animation->sheet->height;

    SDL_RendererFlip flip = SDL_FLIP_NONE;
    flip += animation->flip_x ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
    flip += animation->flip_y ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;

    SDL_Rect clip = {sx, sy, animation->sheet->width, animation->sheet->height};
    engine_texture_render(animation->sheet->image, target_x, target_y, &clip, flip);
}

