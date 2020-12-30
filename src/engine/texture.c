//
// Created by albert on 2/6/20.
//

#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include "engine.h"

Texture *engine_texture_create_from_text(char *text, SDL_Color color) {
    Texture *texture = malloc(sizeof(Texture));
    texture->texture = NULL;

    TTF_Font *gFont = NULL;
    gFont = TTF_OpenFont( "../assets/16_true_type_fonts/retganon.ttf", 16 );
    if(gFont == NULL) {
        printf("Unable to load font. SDL_ttf error: %s\n", TTF_GetError());
        return NULL;
    }
    SDL_Surface *surface = TTF_RenderText_Blended_Wrapped(gFont, text, color, 500);
    if(surface == NULL) {
        printf("Unable to render text surface. SDL_ttf error: %s\n", TTF_GetError());
    } else {
        texture->texture = SDL_CreateTextureFromSurface(engine_texture_global_renderer, surface);
        if(texture == NULL) {
            printf("Unable to create texture from rendered text. SDL_Error: %s\n", TTF_GetError());
        } else {
            texture->width = surface->w;
            texture->height = surface->h;
        }

        SDL_FreeSurface(surface);

        return texture;
    }
    return NULL;
}

Texture *engine_texture_create(char *path) {
    Texture *texture = malloc(sizeof(Texture));
    texture->texture = NULL;

    SDL_Surface *loaded_surface = IMG_Load(path);

    if(loaded_surface == NULL) {
        printf("Unable to load image %s. IMG Error: %s\n", path, IMG_GetError());
        return NULL;
    }

    texture->texture = SDL_CreateTextureFromSurface(engine_texture_global_renderer, loaded_surface);
    if(texture->texture == NULL) {
        printf("Unable to create texture %s. SDL Error: %s\n", path, IMG_GetError());
    }

    SDL_FreeSurface(loaded_surface);

    SDL_QueryTexture(texture->texture, NULL, NULL, &texture->width, &texture->height);

    return texture;
}

void engine_texture_render(Texture *texture, int x, int y, SDL_Rect* clip, SDL_RendererFlip flip) {
    SDL_Rect render_quad = {x, y, texture->width, texture->height};
    if(clip != NULL) {
        render_quad.w = clip->w;
        render_quad.h = clip->h;
    }

    SDL_RenderCopyEx(engine_texture_global_renderer, texture->texture, clip, &render_quad, 0, NULL, flip);
}

void engine_texture_destroy(Texture *texture) {
    SDL_DestroyTexture(texture->texture);
    free(texture);
}
