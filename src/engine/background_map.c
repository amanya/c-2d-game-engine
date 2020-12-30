//
// Created by albert on 10/5/20.
//

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <allegro5/color.h>
#include <allegro5/bitmap.h>
#include <allegro5/haptic.h>
#include <allegro5/allegro.h>
#include "engine.h"

LayerMap *engine_map_layer_create(int tile_size, const char *layer_name, Tileset *tileset) {
    LayerMap *layer_map = malloc(sizeof(LayerMap));
    layer_map->tile_size = tile_size;
    layer_map->tileset = tileset;
    layer_map->layer_name = layer_name;
    layer_map->enabled = true;
    layer_map->distance = 1;
    layer_map->repeat = false;
    layer_map->scroll_x = 0;
    layer_map->scroll_y = 0;
    layer_map->foreground = false;
    layer_map->pre_render = true;
    layer_map->chunk_size = 512;
    layer_map->chunks = NULL;
    layer_map->width = (int)tileset->map->width;
    layer_map->height = (int)tileset->map->height;

    return layer_map;
}

void engine_map_layer_destroy(LayerMap *map_layer) {
    free(map_layer);
}

tmx_layer* engine_map_layer_find_layer(tmx_map *map, const char *name) {
    tmx_layer *temp;

    if (map->ly_head == NULL) {
        printf("Map is empty");
        exit(1);
    }

    temp = map->ly_head;

    while(strcmp(temp->name, name) != 0) {
        temp = temp->next;
    }

    return temp;
}

void engine_map_layer_draw_tile(void *image, int sx, int sy, int sw, int sh,
                                int dx, int dy, unsigned int flags) {
    if (dx < 0) {
        int delta = 0 - dx;
        sx += delta;
        sw -= delta;
        dx = 0;
    }
    if (dy < 0) {
        int delta = 0 - dy;
        sy += delta;
        sh -= delta;
        dy = 0;
    }

    SDL_Rect src_rect, dst_rect;
    src_rect.x = sx;
    src_rect.y = sy;
    src_rect.w = dst_rect.w = sw;
    src_rect.h = dst_rect.h = sh;
    dst_rect.x = dx;
    dst_rect.y = dy;
    SDL_RenderCopy(engine_texture_global_renderer, (SDL_Texture *)image, &src_rect, &dst_rect);
}

void engine_map_layer_screen_pos(LayerMap *layer_map, int x, int y) {
    layer_map->scroll_x = x / layer_map->distance;
    layer_map->scroll_y = y / layer_map->distance;
}

void engine_map_layer_draw(LayerMap *layer_map, System *system) {
    if (!layer_map->enabled) {
        return;
    }
    if (layer_map->pre_render) {
        //engine_map_layer_draw_pre_rendered(layer_map, system);
        engine_map_layer_draw_tiled(layer_map, system);
    }
    else {
        engine_map_layer_draw_tiled(layer_map, system);
    }
}

void engine_map_layer_draw_tiled(LayerMap *layer_map, System *system) {
    if (!layer_map->enabled) {
        return;
    }
    tmx_layer *layer = engine_map_layer_find_layer(layer_map->tileset->map, layer_map->layer_name);

    int tile = 0;
    int tile_offset_x = layer_map->scroll_x / layer_map->tile_size;
    int tile_offset_y = layer_map->scroll_y / layer_map->tile_size;
    int px_offset_x = layer_map->scroll_x % layer_map->tile_size;
    int px_offset_y = layer_map->scroll_y % layer_map->tile_size;
    int px_min_x = -px_offset_x - (int)layer_map->tile_size;
    int px_min_y = -px_offset_y - (int)layer_map->tile_size;
    int px_max_x = system->width - layer_map->tile_size - px_offset_x;
    int px_max_y = system->height - layer_map->tile_size - px_offset_y;

    px_max_y += layer_map->tile_size; // draw extra tile at the bottom to prevent flickering

    int map_y = -1;
    for (int px_y = px_min_y; px_y <= px_max_y; px_y += layer_map->tile_size) {
        map_y++;
        int tile_y = map_y + tile_offset_y;

        // Repeat Y?
        if (tile_y >= system->height || tile_y < 0) {
            if (!layer_map->repeat) { continue; }
            tile_y = ((tile_y % system->height) + system->height) % system->height;
        }

        int map_x = -1;
        for (int px_x = px_min_x; px_x <= px_max_x; px_x += layer_map->tile_size) {
            map_x ++;
            int tile_x = map_x + tile_offset_x;

            // Repeat X"?
            if (tile_x >= system->width || tile_x < 0) {
                if (!layer_map->repeat) { continue; }
                tile_x = ((tile_x % system->width) + system->width) % system->width;
            }

            // Draw!
            tile = layer->content.gids[(tile_y * layer_map->tileset->map->width) + tile_x] & TMX_FLIP_BITS_REMOVAL;
            if (tile > 256 || tile < 0) {
                tile = 0;
            }
            if (tile) {
                void *image;
                u_int flags;
                tmx_image *im = layer_map->tileset->map->tiles[tile]->image;
                tmx_tileset *ts = layer_map->tileset->map->tiles[tile]->tileset;
                u_int sx = layer_map->tileset->map->tiles[tile]->ul_x;
                u_int sy = layer_map->tileset->map->tiles[tile]->ul_y;
                u_int w = ts->tile_width;
                u_int h = ts->tile_height;
                if (im) {
                    image = im->resource_image;
                }
                else {
                    image = ts->image->resource_image;
                }
                flags = layer->content.gids[(tile_y * layer_map->tileset->map->width) + tile_x & ~TMX_FLIP_BITS_REMOVAL];
                engine_map_layer_draw_tile(image, sx, sy, w, h, px_x + w, px_y + h, flags);
            }
        }
    }
}

/*
void engine_map_layer_draw_pre_rendered(LayerMap *layer_map, System *system) {
    if (!layer_map->chunks) {
        engine_map_layer_pre_render_map_to_chunks(layer_map, system);
    }

    int dx = engine_system_get_draw_pos(system, layer_map->scroll_x);
    int dy = engine_system_get_draw_pos(system, layer_map->scroll_y);

    if (layer_map->repeat) {
        int w = layer_map->width * layer_map->tile_size * system->scale;
        dx = (dx % w + w) % w;
        int h = layer_map->height * layer_map->tile_size * system->scale;
        dy = (dy % h + h) % h;
    }

    int val1 = (int)floorf((float)dx / (float)layer_map->chunk_size);
    int min_chunk_x = val1 > 0 ? val1 : 0;
    int val2 = (int)floorf((float)dy / (float)layer_map->chunk_size);
    int min_chunk_y = val2 > 0 ? val2 : 0;
    int max_chunk_x = ceilf((float)(dx + system->real_width) / (float)layer_map->chunk_size);
    int max_chunk_y = ceilf((float)(dy + system->real_height) / (float)layer_map->chunk_size);
    int max_real_chunk_x = layer_map->chunk_cols;
    int max_real_chunk_y = layer_map->chunk_rows;

    if (!layer_map->repeat) {
        max_chunk_x = max_chunk_x < max_real_chunk_x ? max_chunk_x : max_real_chunk_x;
        max_chunk_y = max_chunk_y < max_real_chunk_y ? max_chunk_y : max_real_chunk_y;
    }

    int nudge_y = 0;
    ALLEGRO_BITMAP *chunk;
    for (int cy = min_chunk_y; cy < max_chunk_y; cy++) {
        int nudge_x = 0;
        for (int cx = min_chunk_x; cx < max_chunk_x; cx++) {
            chunk = layer_map->chunks[(cy % max_real_chunk_y) * layer_map->chunk_cols + (cx % max_chunk_x)];
            int x = -dx + cx * layer_map->chunk_size - nudge_x;
            int y = -dy + cy * layer_map->chunk_size - nudge_y;
            al_draw_bitmap(chunk, x, y, 0);

            int chunk_width = al_get_bitmap_width(chunk);

            if (layer_map->repeat && chunk_width < layer_map->chunk_size && x + chunk_width < system->real_width) {
                nudge_x += layer_map->chunk_size - chunk_width;
                if (cy == min_chunk_y) {
                    max_chunk_x++;
                }
            }
        }
        int chunk_height = al_get_bitmap_height(chunk);
        int y = -dy + cy * layer_map->chunk_size - nudge_y;
        if (layer_map->repeat && chunk_height < layer_map->chunk_size && y + chunk_height < system->real_height) {
            nudge_y += layer_map->chunk_size - chunk_height;
            max_chunk_y++;
        }
    }
}


ALLEGRO_BITMAP *engine_map_layer_pre_render_chunk(LayerMap *layer_map, System *system, int cx, int cy, int w, int h) {
    int tw = w / layer_map->tile_size / system->scale + 1;
    int th = h / layer_map->tile_size / system->scale + 1;

    int nx = (cx * layer_map->chunk_size / system->scale) % layer_map->tile_size;
    int ny = (cy * layer_map->chunk_size / system->scale) % layer_map->tile_size;

    int tx = (int)floorf((float)cx * (float)layer_map->chunk_size / (float)layer_map->tile_size / (float)system->scale);
    int ty = (int)floorf((float)cy * (float)layer_map->chunk_size / (float)layer_map->tile_size / (float)system->scale);

    ALLEGRO_BITMAP *chunk = al_create_bitmap(w, h);
    ALLEGRO_BITMAP *restore = al_get_target_bitmap();
    al_set_target_bitmap(chunk);

    tmx_layer *layer = engine_map_layer_find_layer(layer_map->tileset->map, layer_map->layer_name);

    for (int x = 0; x < tw; x++) {
        for (int y = 0; y < th; y++) {
            if (x + tx < layer_map->width && y + ty < layer_map->height) {
                int tile = layer->content.gids[((y+ty) * layer_map->tileset->map->width) + x+tx];
                if (tile) {
                    tmx_tileset *ts = layer_map->tileset->map->tiles[tile]->tileset;
                    void *image = ts->image->resource_image;
                    u_int sx = layer_map->tileset->map->tiles[tile]->ul_x;
                    u_int sy = layer_map->tileset->map->tiles[tile]->ul_y;
                    u_int w = ts->tile_width;
                    u_int h = ts->tile_height;
                    u_int flags = tile & ~TMX_FLIP_BITS_REMOVAL;
                    engine_map_layer_draw_tile(image, sx, sy, w, h, x * layer_map->tile_size - nx, y * layer_map->tile_size - ny, flags);
                }
            }
        }
    }
    al_set_target_bitmap(restore);
    return chunk;
}

void engine_map_layer_pre_render_map_to_chunks(LayerMap *layer_map, System *system) {
    int total_width = layer_map->width * layer_map->tile_size * system->scale;
    int total_height = layer_map->height * layer_map->tile_size * system->scale;

    int max_dimension = (total_width > total_height ? total_width : total_height);
    layer_map->chunk_size = max_dimension > layer_map->chunk_size ? layer_map->chunk_size : max_dimension;

    int chunk_cols = (int)ceilf((float)total_width / (float)layer_map->chunk_size);
    int chunk_rows = (int)ceilf((float)total_height / (float)layer_map->chunk_size);
    layer_map->chunk_cols = chunk_cols;
    layer_map->chunk_rows = chunk_rows;

    layer_map->chunks = malloc(sizeof(ALLEGRO_BITMAP*) * chunk_cols * chunk_rows);

    for (int y=0; y < chunk_rows; y++) {
        for (int x=0; x < chunk_cols; x++) {
            int chunk_width = x == chunk_cols - 1 ? total_width - x * layer_map->chunk_size : layer_map->chunk_size;
            int chunk_height = y == chunk_rows - 1 ? total_height - y * layer_map->chunk_size : layer_map->chunk_size;

            layer_map->chunks[y * chunk_cols + x] = engine_map_layer_pre_render_chunk(layer_map, system, x, y, chunk_width, chunk_height);
        }
    }
}
*/
