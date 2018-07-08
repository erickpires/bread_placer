#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

#include <SDL2/SDL.h>


#include "draw.h"

static void save_texture(SDL_Renderer*, SDL_Texture*, const char *);

DrawData init_SDL() {
    DrawData result;

    if(SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        fprintf(stderr, "Failed to init SDL.");
        exit(5);
    }

    TTF_Init();

    SDL_DisplayMode DM;
    SDL_GetCurrentDisplayMode(0, &DM);
    result.width = DM.w;
    result.height = DM.h;

    result.window = SDL_CreateWindow("Bread Placer", SDL_WINDOWPOS_UNDEFINED,
                                     SDL_WINDOWPOS_UNDEFINED, result.width,
                                     result.height, SDL_WINDOW_SHOWN);

    if (!result.window) {
        fprintf(stderr, "Failed to open window.");
        exit(5);
    }

    result.renderer = SDL_CreateRenderer(result.window, -1,
                                         SDL_RENDERER_ACCELERATED |
                                         SDL_RENDERER_TARGETTEXTURE);
    result.canvas = SDL_CreateTexture(result.renderer, SDL_PIXELFORMAT_RGBA32,
                                      SDL_TEXTUREACCESS_TARGET,
                                      CANVAS_WIDTH, CANVAS_HEIGHT);

    SDL_GL_SetSwapInterval(1);
    SDL_SetWindowFullscreen(result.window, SDL_WINDOW_FULLSCREEN);

    result.text_color.r = 0x00;
    result.text_color.b = 0x00;
    result.text_color.g = 0x00;
    result.text_color.a = 0xff;

    result.white_color.r = 0xff;
    result.white_color.b = 0xff;
    result.white_color.g = 0xff;
    result.white_color.a = 0xff;

    result.clear_sans = TTF_OpenFont("ClearSans-Medium.ttf", TEXT_FONT_SIZE);
    result.clear_sans_bold = TTF_OpenFont("ClearSans-Bold.ttf", TEXT_FONT_SIZE);

    return result;
}

static void draw_text(SDL_Renderer* renderer, TTF_Font* font, SDL_Color color,
                      char* text, int x0, int y0) {
    SDL_Surface* text_surf = TTF_RenderText_Solid(font, text, color);
    SDL_Texture* text_texture = SDL_CreateTextureFromSurface(renderer, text_surf);

    int w, h;
    SDL_QueryTexture(text_texture, NULL, NULL, &w, &h);
    SDL_Rect dest_rect = {.x = x0, .y = y0, .w = w, .h = h};

    SDL_RenderCopy(renderer, text_texture, NULL, &dest_rect);

    SDL_FreeSurface(text_surf);
    SDL_DestroyTexture(text_texture);
}

static void draw_vertical_line_at(SDL_Renderer* renderer, int x) {
    int x0 = x - LINE_WIDTH / 2;
    int y0 = 0;
    int w = LINE_WIDTH;
    int h = CANVAS_HEIGHT;
    SDL_Rect rect = {.x = x0, .y = y0, .w = w, .h = h};

    SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xff);
    SDL_RenderFillRect(renderer, &rect);
}

static void draw_horizontal_line_at(SDL_Renderer* renderer, int y) {
    int x0 = 0;
    int y0 = y - LINE_WIDTH / 2;
    int w = CANVAS_WIDTH;
    int h = LINE_WIDTH;
    SDL_Rect rect = {.x = x0, .y = y0, .w = w, .h = h};

    SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xff);
    SDL_RenderFillRect(renderer, &rect);
}


static void draw_grid(SDL_Renderer* renderer) {
    int vertical_stride = CANVAS_HEIGHT / 63;

    int current_y = vertical_stride;
    for(uint i = 1;
        i < 64;
        i++, current_y += vertical_stride)
    {
        draw_horizontal_line_at(renderer, current_y);
    }

    int number_cell_width = vertical_stride;
    int remaining_space = CANVAS_WIDTH - 4 * number_cell_width;
    int large_cell_width = remaining_space / 9;

    // NOTE(erick): I'm too lazy to think in a loop for these.
    int current_x = number_cell_width;
    draw_vertical_line_at(renderer, current_x);

    current_x += large_cell_width;
    draw_vertical_line_at(renderer, current_x);

    current_x += large_cell_width;
    draw_vertical_line_at(renderer, current_x);

    current_x += large_cell_width;
    draw_vertical_line_at(renderer, current_x);

    current_x += number_cell_width;
    draw_vertical_line_at(renderer, current_x);

    current_x += large_cell_width;
    draw_vertical_line_at(renderer, current_x);

    current_x += large_cell_width;
    draw_vertical_line_at(renderer, current_x);

    current_x += large_cell_width;
    draw_vertical_line_at(renderer, current_x);

    current_x += number_cell_width;
    draw_vertical_line_at(renderer, current_x);

    current_x += large_cell_width;
    draw_vertical_line_at(renderer, current_x);

    current_x += large_cell_width;
    draw_vertical_line_at(renderer, current_x);

    current_x += large_cell_width;
    draw_vertical_line_at(renderer, current_x);
}

void draw_numbers(DrawData* data) {
    int vertical_stride = CANVAS_HEIGHT / 63;
    int number_cell_width = vertical_stride;

    int remaining_space = CANVAS_WIDTH - 4 * number_cell_width;
    int large_cell_width = remaining_space / 9;

    int horizontal_stride = 3 * large_cell_width + number_cell_width;

    char buffer[4];
    int current_y = -5;
    for(uint i = 1; i <= 64; i++, current_y += vertical_stride) {
        sprintf(buffer, "%2d", i);

        int current_x = 5;
        for(uint j = 0; j < 4; j++, current_x += horizontal_stride) {
            draw_text(data->renderer, data->clear_sans, data->text_color, buffer,
                      current_x, current_y);
        }
    }
}

void draw_breadboard(DrawData* data) {
    // Attach the canvas;
    SDL_SetRenderTarget(data->renderer, data->canvas);
    SDL_SetRenderDrawColor(data->renderer, 0xff, 0xff, 0xff, 0xff);
    SDL_RenderClear(data->renderer);

    draw_grid(data->renderer);
    draw_numbers(data);

    // Detach the canvas;
    SDL_SetRenderTarget(data->renderer, NULL);
    SDL_SetRenderDrawColor(data->renderer, 0xff, 0x00, 0xff, 0xff);
    SDL_RenderClear(data->renderer);

    char buffer[256];
    sprintf(buffer, "FPS: %.2f", 1.0 / data->dt);
    draw_text(data->renderer, data->clear_sans, data->white_color, buffer, 0, 0);

    SDL_Rect dest_rect;
    dest_rect.h = data->height;
    dest_rect.w = width_preserve_ratio(data->height);
    dest_rect.y = 0;
    dest_rect.x = (data->width - dest_rect.w) / 2;

    SDL_RenderCopyEx(data->renderer, data->canvas,
                     NULL, &dest_rect, 0, NULL, SDL_FLIP_NONE);

    // int SDL_RenderCopyEx(SDL_Renderer*          renderer,
    //                  SDL_Texture*           texture,
    //                  const SDL_Rect*        srcrect,
    //                  const SDL_Rect*        dstrect,
    //                  const double           angle,
    //                  const SDL_Point*       center,
    //                  const SDL_RendererFlip flip)

}

void swap_buffers(DrawData* data) {
    SDL_RenderPresent(data->renderer);
}

void save_image(DrawData* data) {
    save_texture(data->renderer, data->canvas, "output.bmp");
}

// NOTE(erick): Copy-pasta from here:
// https://stackoverflow.com/questions/34255820/save-sdl-texture-to-file

static void save_texture(SDL_Renderer* renderer,
                         SDL_Texture* texture,
                         const char* filename) {
    uint32 format;
    int w, h;

    /* Get information about texture we want to save */
    int result = SDL_QueryTexture(texture, &format, NULL, &w, &h);
    if (result != 0) {
        fprintf(stderr, "Failed querying texture: %s\n", SDL_GetError());
        return;
    }

    int bytes_per_pixel = SDL_BYTESPERPIXEL(format);

    /* Create buffer to hold texture data and load it */
    void* pixels = malloc(w * h * bytes_per_pixel);
    if (!pixels) {
        fprintf(stderr, "Failed allocating memory\n");
        return;
    }

    SDL_SetRenderTarget(renderer, texture);
    result = SDL_RenderReadPixels(renderer, NULL, format, pixels, w * bytes_per_pixel);
    if (result != 0) {
        fprintf(stderr, "Failed reading pixel data: %s\n", SDL_GetError());
        free(pixels);
        return;
    }

    /* Copy pixel data over to surface */
    SDL_Surface* surf = SDL_CreateRGBSurfaceWithFormatFrom(pixels, w, h,
                                                           SDL_BITSPERPIXEL(format),
                                                           w * bytes_per_pixel,
                                                           format);
    if (!surf) {
        fprintf(stderr, "Failed creating new surface: %s\n", SDL_GetError());
        goto cleanup;
    }

    /* Save result to an image */
    result = SDL_SaveBMP(surf, filename);
    if (result != 0) {
        fprintf(stderr, "Failed saving image: %s\n", SDL_GetError());
        goto cleanup;
    }

    fprintf(stderr, "Saved texture as BMP to \"%s\"\n", filename);

cleanup:
    SDL_SetRenderTarget(renderer, NULL);
    SDL_FreeSurface(surf);
    free(pixels);
}
