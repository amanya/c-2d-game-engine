//
// Created by albert on 10/5/20.
//

#include <stdlib.h>
#include <math.h>
#include "engine.h"

// -- TILEDEF ---

#define H 3/2.0
#define N 1/3.0
#define M 2/3.0
#define SOLID true
#define NON_SOLID false

Tiledef default_tiledef[] = {
        [12]={0, 0, 1, 0, NON_SOLID}, // Go N
        [23]={1, 1, 0, 1, NON_SOLID}, // Go S
        [34]={1, 0, 1, 1, NON_SOLID}, // Go E
        [45]={0, 1, 0, 0, NON_SOLID}, // Go W
};

#undef NON_SOLID
#undef SOLID
#undef M
#undef N
#undef H

CollisionMap *engine_collision_map_create(u_int tile_size, Tiledef *tiledef, LayerMap *layer_map) {
    CollisionMap *collision_map = malloc(sizeof(CollisionMap));
    collision_map->tile_size = tile_size;
    collision_map->tiledef = tiledef ? tiledef : default_tiledef;
    tmx_layer *layer = engine_map_layer_find_layer(layer_map->tileset->map, "Collision");
    collision_map->data = layer->content.gids;
    collision_map->width = layer_map->tileset->map->width;
    collision_map->height = layer_map->tileset->map->height;
    return collision_map;
}

void engine_collision_map_destroy(CollisionMap *collision_map) {
    free(collision_map);
}

void engine_collision_map_trace(CollisionMap *collision_map, TraceResult *res, int x, int y, float vx, float vy, int width, int height) {
    res->collision_x = false;
    res->collision_y = false;
    res->collision_slope = false;
    res->pos_x = x;
    res->pos_y = y;
    res->tile_x = 0;
    res->tile_y = 0;

    float fvx = fabsf(vx);
    float fvy = fabsf(vy);
    float val1 = fvy > fvx ? fvy : fvx;
    int steps = (int)ceilf((val1 + (float)0.1) / (float)collision_map->tile_size);

    if (steps > 1) {
        float sx = vx / (float)steps;
        float sy = vy / (float)steps;

        for (int i = 0; i < steps && (sx || sy); i++) {
            engine_collision_map_trace_step(collision_map, res, x, y, sx, sy, width, height, vx, vy, i);
            x = res->pos_x;
            y = res->pos_y;
            if (res->collision_x) { sx = 0; vx = 0; }
            if (res->collision_y) { sy = 0; vy = 0; }
            if (res->collision_slope) { break; }
        }
    }
    else {
        engine_collision_map_trace_step(collision_map, res, x, y, vx, vy, width, height, vx, vy, 0);
    }
}

bool engine_collision_map_check_tile_def(CollisionMap *collision_map, TraceResult *res, int t, int x, int y, float vx, float vy, int width, int height, int tile_x, int tile_y) {
    Tiledef def = collision_map->tiledef[t];

    float lx = (float)((tile_x + def.x1) * collision_map->tile_size);
    float ly = (float)((tile_y + def.y1) * collision_map->tile_size);
    float lvx = (float)((def.x2 - def.x1) * collision_map->tile_size);
    float lvy = (float)((def.y2 - def.y1) * collision_map->tile_size);
    bool solid = def.solid;

    float tx = (float)x + vx + (lvy < 0 ? (float)width : 0) - lx;
    float ty = (float)y + vy + (lvx > 0 ? (float)height : 0) - ly;

    if (lvx * ty - lvy * tx > 0) {
        if (vx * -lvy + vy * lvx < 0) {
            return solid;
        }

        float length = sqrtf(lvx * lvx + lvy * lvy);
        float nx = lvy / length;
        float ny = -lvx / length;

        float proj = tx * nx + ty * ny;
        float px = nx * proj;
        float py = ny * proj;

        if (px * px + py * py >= vx * vx + vy * vy) {
            float val = lvx * (ty - vy) - lvy * (tx - vx);
            return solid || val < 0.5f;
        }

        res->pos_x = (int)((float)x + vx - px);
        res->pos_y = (int)((float)y + vy - py);
        res->collision_slope = true;

        return true;
    }
    return false;
}

void engine_collision_map_trace_step(CollisionMap *collision_map, TraceResult *res, int x, int y, float vx, float vy, int width, int height, float rvx, float rvy, int step) {
    res->pos_x += (int)vx;
    res->pos_y += (int)vy;

    int32_t t;

    // Horizontal collision walls
    if (vx) {
        float px_offset_x = vx > 0 ? (float)width : 0;
        float tile_offset_x = vx < 0 ? (float)collision_map->tile_size : 0;

        float val1 = floorf((float)y / (float)collision_map->tile_size);
        int first_tile_y = val1 > 0 ? (int)val1 : 0;

        float val2 = ceilf((float)(y + height) / (float)collision_map->tile_size);
        int last_tile_y = val2 < (float)collision_map->height ? (int)val2 : collision_map->height;

        int tile_x = (int)floorf(((float)res->pos_x + px_offset_x) / (float)collision_map->tile_size);

        int prev_tile_x = (int)floorf(((float)x + px_offset_x) / (float)collision_map->tile_size);

        if (step == 0 || tile_x == prev_tile_x || prev_tile_x < 0 || prev_tile_x >= collision_map->width) {
            prev_tile_x = -1;
        }

        if (tile_x >= 0 && tile_x < collision_map->width) {
            for (int tile_y = first_tile_y; tile_y < last_tile_y; tile_y++) {
                if (prev_tile_x != -1) {
                    t = collision_map->data[tile_y * collision_map->width + prev_tile_x];
                    if (
                            t > 1 &&
                            engine_collision_map_check_tile_def(collision_map, res, t, x, y, rvx, rvy, width, height,
                                                                prev_tile_x, tile_y)
                            ) {
                        break;
                    }
                }

                t = collision_map->data[tile_y * collision_map->width + tile_x];
                if (
                        t == 1 ||
                        (t > 1 && engine_collision_map_check_tile_def(collision_map, res, t, x, y, rvx, rvy, width, height, tile_x, tile_y))
                        ) {
                    if ( t > 1 && res->collision_slope ) {
                        break;
                    }

                    res->collision_x = true;
                    res->tile_x = t;
                    x = res->pos_x = tile_x * collision_map->tile_size - (int)px_offset_x + (int)tile_offset_x;
                    rvx = 0;
                    break;
                }
            }
        }
    }

    if ( vy ) {
        float px_offset_y = vy > 0 ? (float)height : 0;
        float tile_offset_y = vy < 0 ? (float)collision_map->tile_size : 0;

        float val1 = floorf((float)x / (float)collision_map->tile_size);
        int first_tile_x = val1 > 0 ? (int)val1 : 0;

        float val2 = ceilf((float)(x + width) / (float)collision_map->tile_size);
        int last_tile_x = val2 < (float)collision_map->width ? (int)val2 : collision_map->width;

        int tile_y = (int)floorf(((float)res->pos_y + px_offset_y) / (float)collision_map->tile_size);
        int prev_tile_y = (int)floorf((float)y + px_offset_y / (float)collision_map->tile_size);

        if (step == 0 || tile_y == prev_tile_y || prev_tile_y < 0 || prev_tile_y >= collision_map->height) {
            prev_tile_y = -1;
        }

        if (tile_y >= 0 && tile_y < collision_map->height) {
            for (int tile_x = first_tile_x; tile_x < last_tile_x; tile_x++) {
                if (prev_tile_y != -1) {
                    t = collision_map->data[prev_tile_y * collision_map->width + tile_x];
                    if (
                            t > 1 &&
                            engine_collision_map_check_tile_def(collision_map, res, t, x, y, rvx, rvy, width, height, tile_x,
                                                                prev_tile_y)
                            ) {
                        break;
                    }
                }

                t = collision_map->data[tile_y * collision_map->width + tile_x];
                if (
                        t == 1 ||
                        (t > 1 && engine_collision_map_check_tile_def(collision_map, res, t, x, y, rvx, rvy, width, height, tile_x, tile_y))
                        ) {
                    if ( t > 1 && res->collision_slope ) {
                        break;
                    }

                    res->collision_y = true;
                    res->tile_y = t;
                    res->pos_y = tile_y * collision_map->tile_size - (int)px_offset_y + (int)tile_offset_y;
                    break;
                }
            }
        }
    }
}

