//
// Created by albert on 10/5/20.
//

#include <stdlib.h>
#include "engine.h"

AnimationSheet *engine_animation_sheet_create(unsigned int width, unsigned int height, Texture *texture) {
    AnimationSheet *as = malloc(sizeof(AnimationSheet));
    as->width = width;
    as->height = height;
    as->image = texture;
    return as;
}

void engine_animation_sheet_destroy(AnimationSheet *animation_sheet) {
    free(animation_sheet);
}

