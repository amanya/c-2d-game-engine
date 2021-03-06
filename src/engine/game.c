//
// Created by albert on 10/5/20.
//

#include <stdlib.h>
#include <stdio.h>
#include "engine.h"
#include "../libs/list/list.h"

Game *engine_game_create() {
    Game *game = malloc(sizeof(Game));
    game->r_screen_x = 0;
    game->r_screen_y = 0;
    game->screen_x = 0;
    game->screen_y = 0;
    game->parallax_map = NULL;
}

void engine_game_destroy(Game *game) {
    free(game);
}

void engine_game_run(Game *game) {
    engine_game_update(game);
    engine_game_draw(game);
}

void engine_game_update(Game *game) {
    engine_game_update_entities(game);
    engine_game_check_entities(game);

    int screen_width = game->system->width;
    int screen_height = game->system->height;

    int dx = screen_width / 2;
    int dy = screen_height / 2;

    int cx1 = game->screen_x;
    int cx2 = game->screen_x + screen_width;
    int cy1 = game->screen_y;
    int cy2 = game->screen_y + screen_height;

    if (cx1 < 0) {
        cy1 = 0;
        cy2 = screen_width;
    }

    if (cx2 > game->map_width) {
        cx1 = game->map_width - screen_width;
        cx2 = game->map_width;
    }

    if (cy1 < 0) {
        cy1 = 0;
        cy2 = screen_height;
    }

    if (cy2 > game->map_height) {
        cy1 = game->map_height - screen_height;
        cy2 = game->map_height;
    }

    int x = game->camera_ent->pos_x;
    int y = game->camera_ent->pos_y;

    int newsx = game->screen_x;
    int newsy = game->screen_y;

    if (newsx < cx1 + dx) {
        newsx = x - dx;
    } else if (newsx > cx2 - dx) {
        newsx = x + dx - screen_width;
    }
    if (newsx < 0) newsx = 0;
    else if (newsx > game->map_width - screen_width) newsx = game->map_width - screen_width;

    if (newsy < cy1 + dy) {
        newsy = y - dy;
    } else if (newsy > cy2 - dy) {
        newsy = y + dy - screen_height;
    }
    if (newsy < 0) newsy = 0;
    else if (newsy > game->map_height - screen_height) newsy = game->map_height - screen_height;

    int delta_x = newsx - game->screen_x;
    int delta_y = newsy - game->screen_y;
    if (delta_y > 2) {
        newsy = game->screen_y + 2;
    } else if (delta_y < -2) {
        newsy = game->screen_y - 2;
    }

    game->screen_x = newsx;
    game->screen_y = newsy;
}

void engine_game_draw(Game *game) {
    if (game->clear_color) {
        engine_system_clear(game->layer_maps[0]->tileset->map->backgroundcolor);
    }

    game->r_screen_x = engine_system_get_draw_pos(game->system, game->screen_x);
    game->r_screen_y = engine_system_get_draw_pos(game->system, game->screen_y);

    engine_parallax_map_draw(game->parallax_map, game);

    for(int map_index = 0; map_index < game->layer_maps_l; map_index++) {
        LayerMap *lm = game->layer_maps[map_index];
        if(!lm->foreground) {
            engine_map_layer_screen_pos(lm, game->screen_x, game->screen_y);
            engine_map_layer_draw(lm, game->system);
        }
    }

    engine_game_draw_entities(game);

    for(int map_index = 0; map_index < game->layer_maps_l; map_index++) {
        LayerMap *lm = game->layer_maps[map_index];
        if(lm->foreground) {
            engine_map_layer_screen_pos(lm, game->screen_x, game->screen_y);
            engine_map_layer_draw(lm, game->system);
        }
    }

    // engine_game_draw_debug(game);
}

void engine_game_update_entities(Game *game) {
    for (int i = 0; i < game->entities_l; i++ ) {
        Entity *ent = game->entities[i];
        if (!ent->killed) {
            engine_entity_update(ent, game->collision_map);
        }
    }
}

void engine_game_draw_entities(Game *game) {
    for (int i = 0; i < game->entities_l; i++) {
        engine_entity_draw(game->entities[i], game);
    }
}

void engine_game_draw_debug(Game *game) {
    for (int i = 0; i < game->entities_l; i++) {
        Entity *ent = game->entities[i];
        SDL_Rect r = {ent->pos_x - game->r_screen_x, ent->pos_y - game->r_screen_y, ent->size_x, ent->size_y};
        SDL_SetRenderDrawColor(engine_texture_global_renderer, 0, 0, 0, 0);
        SDL_RenderDrawRect(engine_texture_global_renderer, &r);
    }
}

void engine_game_check_entities(Game *game) {
    List entities;
    List entities_checked;
    list_init(&entities, NULL);
    list_init(&entities_checked, NULL);

    for (int e=0; e < game->entities_l; e++) {
        Entity *entity = game->entities[e];

        if (
                entity->type == ENTITY_TYPE_NONE &&
                entity->check_against == ENTITY_TYPE_NONE &&
                entity->collides == ENTITY_COLLIDES_NEVER
                ) {
            continue;
        }

        list_ins_next(&entities, NULL, entity);
    }

    if (list_size(&entities) == 0) return;

    for (int e=0; e < game->entities_l; e++) {
        Entity *entity = game->entities[e];

        ListItem *item = list_head(&entities);
        do {
            if (entity == list_data(item)) {
                item = list_next(item);
                continue;
            }

            if (engine_entity_touches(entity, list_data(item)) && !list_is_member(&entities_checked, item) ) {
                list_ins_next(&entities_checked, NULL, entity);
                engine_entity_check_pair(entity, item->data);
            }

            item = list_next(item);
        } while (item);
    }

    list_destroy(&entities);
    list_destroy(&entities_checked);
}

