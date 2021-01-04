//
// Created by albert on 3/1/21.
//

#include "engine.h"

ParallaxLayer *engine_parallax_layer_create(const char *image_path, float distance) {
    ParallaxLayer *layer = malloc(sizeof(ParallaxLayer));
    Texture *texture = engine_texture_create((char *)image_path);
    layer->image = texture;
    layer->distance = distance;
    layer->offset_x = 0;
    layer->offset_y = 0;
    layer->scroll_x = 0;
    layer->scroll_y = 0;
    return layer;
}

void engine_parallax_layer_destroy(ParallaxLayer *parallax_layer) {
    engine_texture_destroy(parallax_layer->image);
    free(parallax_layer);
}

void engine_parallax_layer_draw(ParallaxLayer *layer, Game *game) {
    int nx = layer->image->width / game->system->width;

    for (int ntx=0; ntx <= nx; ntx++) {
        int px_offset_x = layer->scroll_x % layer->image->width;
        int px_offset_y = layer->scroll_y % layer->image->height;
        int px_min_x = -px_offset_x + layer->offset_x + (ntx * layer->image->width);
        int px_min_y = -px_offset_y + layer->offset_y;

        SDL_Rect src_rect;
        src_rect.x = 0;
        src_rect.y = 0;
        src_rect.w = layer->image->width;
        src_rect.h = layer->image->height;
        engine_texture_render(layer->image, px_min_x, px_min_y, &src_rect, false);
    }

}

void engine_parallax_layer_screen_pos(ParallaxLayer *layer, int x, int y) {
    layer->scroll_x = (int)((float)x / layer->distance);
    layer->scroll_y = (int)((float)y / layer->distance);
}

ParallaxMap *engine_parallax_map_create() {
    ParallaxMap *map = malloc(sizeof(ParallaxMap));
    map->layers_l = 0;
    map->layers = NULL;
    return map;
}

void engine_parallax_map_add_layer(ParallaxMap *parallax_map, const char *image_path, float distance) {
    ParallaxLayer *layer = engine_parallax_layer_create(image_path, distance);
    parallax_map->layers_l++;
    if (parallax_map->layers_l == 1) {
        parallax_map->layers = malloc(sizeof(ParallaxLayer *));
    }
    else {
        parallax_map->layers = realloc(parallax_map->layers, sizeof(ParallaxLayer *) * parallax_map->layers_l);
    }
    parallax_map->layers[parallax_map->layers_l - 1] = layer;
}

void engine_parallax_map_draw(ParallaxMap *parallax_map, Game *game) {
    for (int i=0; i<parallax_map->layers_l; i++) {
        engine_parallax_layer_screen_pos(parallax_map->layers[i], game->screen_x, game->screen_y);
        engine_parallax_layer_draw(parallax_map->layers[i], game);
    }
}

void engine_parallax_map_destroy(ParallaxMap *parallax_map) {
    for (int i = 0; i < parallax_map->layers_l; i++) {
        engine_parallax_layer_destroy(parallax_map->layers[i]);
    }
    free(parallax_map->layers);
    free(parallax_map);
}
