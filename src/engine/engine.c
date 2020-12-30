//
// Created by albert on 2/5/20.
//

#include <string.h>
#include "engine.h"

// --- callbacks ---

void* (*engine_callback_img_load_func) (const char *path) = NULL;
void  (*engine_callback_img_free_func) (void *address) = NULL;
int   (*engine_callback_img_with_func) (void *address) = NULL;
void  (*engine_callback_img_draw_region_func) (void *address, float sx, float sy, float sw, float sh, float dx, float dy, int flags) = NULL;
void  (*engine_callback_img_draw_scaled_bitmap_func) (void *address, float sx, float sy, float sw, float sh, float dx, float dy, float dw, float dh, int flags) = NULL;
uint32_t (*engine_callback_get_time) (void) = NULL;
