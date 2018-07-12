#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

#include <SDL2/SDL.h>

#include "draw.h"

typedef enum {
    LEFT,
    RIGHT
} ColumnSide;

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

void draw_grid(DrawData* data) {
    SDL_Renderer* renderer = data->renderer;
    int current_y = VERTICAL_STRIDE;
    for(uint i = 1;
        i < 64;
        i++, current_y += VERTICAL_STRIDE)
    {
        draw_horizontal_line_at(renderer, current_y);
    }

    // NOTE(erick): I'm too lazy to think in a loop for these.
    int current_x = NUMBER_CELL_WIDTH;
    draw_vertical_line_at(renderer, current_x);

    current_x += TEXT_CELL_WIDTH;
    draw_vertical_line_at(renderer, current_x);

    current_x += IC_CELL_WIDTH;
    draw_vertical_line_at(renderer, current_x);

    current_x += TEXT_CELL_WIDTH;
    draw_vertical_line_at(renderer, current_x);

    current_x += NUMBER_CELL_WIDTH;
    draw_vertical_line_at(renderer, current_x);

    current_x += TEXT_CELL_WIDTH;
    draw_vertical_line_at(renderer, current_x);

    current_x += IC_CELL_WIDTH;
    draw_vertical_line_at(renderer, current_x);

    current_x += TEXT_CELL_WIDTH;
    draw_vertical_line_at(renderer, current_x);

    current_x += NUMBER_CELL_WIDTH;
    draw_vertical_line_at(renderer, current_x);

    current_x += TEXT_CELL_WIDTH;
    draw_vertical_line_at(renderer, current_x);

    current_x += IC_CELL_WIDTH;
    draw_vertical_line_at(renderer, current_x);

    current_x += TEXT_CELL_WIDTH;
    draw_vertical_line_at(renderer, current_x);
}

void draw_numbers(DrawData* data) {
    int horizontal_stride = 2 * TEXT_CELL_WIDTH + TEXT_CELL_WIDTH + NUMBER_CELL_WIDTH;

    char buffer[4];
    int current_y = -5;
    for(uint i = 1; i <= 64; i++, current_y += VERTICAL_STRIDE) {
        sprintf(buffer, "%2d", i);

        int current_x = 5;
        for(uint j = 0; j < 4; j++, current_x += horizontal_stride) {
            draw_text(data->renderer, data->clear_sans, data->text_color, buffer,
                      current_x, current_y);
        }
    }
}

static Vec2 text_cell_coord(uint row, uint column, ColumnSide side) {
    Vec2 result;

    result.y = (row - 1) * VERTICAL_STRIDE;
    result.x = 0;

    // NOTE(erick): This is very ugly, cumbersome and spaghetti. But it's fun anyway.
    switch(column) {
    case 3:
        result.x += NUMBER_CELL_WIDTH + 2 * TEXT_CELL_WIDTH + IC_CELL_WIDTH;
    case 2:
        result.x += NUMBER_CELL_WIDTH + 2 * TEXT_CELL_WIDTH + IC_CELL_WIDTH;
    case 1:
        result.x += NUMBER_CELL_WIDTH;
        break;
    default:
        fprintf(stderr, "Invalid IC column (%d).\n", column);
        exit(5);
    }

    if(side == RIGHT) {
        result.x += TEXT_CELL_WIDTH + IC_CELL_WIDTH;
    }

    return result;
}

static uint first_ic_row(IC* ic) {
    BreadboardLocation loc = ic->location;

    if(loc.orientation == UP) {
        return loc.row;
    } else {
        uint ic_height = ic->n_pins / 2;
        return loc.row - ic_height;
    }
}

static Vec2 coord_of_ic(IC* ic, Vec2* pin_one) {
    Vec2 result;

    BreadboardLocation loc = ic->location;

    int first_row = first_ic_row(ic);

    result.y = (first_row - 1) * VERTICAL_STRIDE;
    result.x = 0;

    // NOTE(erick): This is very ugly, cumbersome and spaghetti. But it's fun anyway.
    switch(loc.column) {
    case 3:
        result.x += NUMBER_CELL_WIDTH + 2 * TEXT_CELL_WIDTH + IC_CELL_WIDTH;
    case 2:
        result.x += NUMBER_CELL_WIDTH + 2 * TEXT_CELL_WIDTH + IC_CELL_WIDTH;
    case 1:
        result.x += NUMBER_CELL_WIDTH + TEXT_CELL_WIDTH;
        break;
    default:
        fprintf(stderr, "Invalid IC column (%d).\n", loc.column);
        exit(5);
    }

    if(pin_one) {
        pin_one->y = (loc.row - 1) * VERTICAL_STRIDE;
        pin_one->x = result.x;
        if(loc.orientation == DOWN) {
            pin_one->y -= VERTICAL_STRIDE;
            pin_one->x += IC_CELL_WIDTH - VERTICAL_STRIDE;
        }
    }

    return result;
}

static Vec2 dimensions_of_ic(IC* ic) {
    uint ic_height = ic->n_pins / 2;

    Vec2 result = {.w = IC_CELL_WIDTH, .h = ic_height * VERTICAL_STRIDE};
    return result;
}

static uint pin_number_no_rotation(IC* ic, uint pin) {
    if(ic->location.orientation == UP) { return pin; }

    if(pin <= ic->n_pins / 2) {
        return ic->n_pins / 2 + pin;
    } else {
        return pin - ic->n_pins / 2;
    }
}

void prepare_canvas(DrawData* data) {
    // Attach the canvas;
    SDL_SetRenderTarget(data->renderer, data->canvas);
    SDL_SetRenderDrawColor(data->renderer, 0xff, 0xff, 0xff, 0xff);
    SDL_RenderClear(data->renderer);
}

void draw_ics(DrawData* data, ICList ic_list) {
    for(usize ic_index = 0; ic_index < ic_list.count; ic_index++) {
        IC* ic = ic_list.data + ic_index;

        if(ic->location.column == 0) { continue; }

        Vec2 pin_one;
        Vec2 corner = coord_of_ic(ic, &pin_one);
        Vec2 dimensions = dimensions_of_ic(ic);

        SDL_Rect ic_outside = {.x = corner.x, .y = corner.y,
                               .h = dimensions.h, .w = dimensions.w};
        SDL_Rect ic_inside = {.x = ic_outside.x + 1 * LINE_WIDTH,
                              .y = ic_outside.y + 1 * LINE_WIDTH,
                              .h = ic_outside.h - 2 * LINE_WIDTH,
                              .w = ic_outside.w - 2 * LINE_WIDTH};
        SDL_Rect pin_one_rect = {.x = pin_one.x + 2 * LINE_WIDTH,
                                 .y = pin_one.y + 2 * LINE_WIDTH,
                                 .h = VERTICAL_STRIDE / 2,
                                 .w = VERTICAL_STRIDE / 2};

        SDL_SetRenderDrawColor(data->renderer, 0x00, 0x00, 0x00, 0xff);
        SDL_RenderFillRect(data->renderer, &ic_outside);

        SDL_SetRenderDrawColor(data->renderer, 0xff, 0xff, 0xff, 0xff);
        SDL_RenderFillRect(data->renderer, &ic_inside);

        SDL_SetRenderDrawColor(data->renderer, 0x00, 0x00, 0x00, 0xff);
        SDL_RenderFillRect(data->renderer, &pin_one_rect);

        uint current_row = first_ic_row(ic);
        for(uint pin = 1; pin <= ic->n_pins / 2; pin++, current_row++) {
            uint no_rotation_pin = pin_number_no_rotation(ic, pin);
            uint pin_index = no_rotation_pin - 1;
            Pin* p = ic->pins + pin_index;

            Vec2 text_coord = text_cell_coord(current_row, ic->location.column, LEFT);
            text_coord.x += TEXT_PADDING;

            draw_text(data->renderer, data->clear_sans, data->text_color,
                      p->label, text_coord.x, text_coord.y);
        }

        current_row--;
        for(uint pin = ic->n_pins / 2 + 1; pin <= ic->n_pins; pin++, current_row--) {
            uint no_rotation_pin = pin_number_no_rotation(ic, pin);
            uint pin_index = no_rotation_pin - 1;
            Pin* p = ic->pins + pin_index;

            Vec2 text_coord = text_cell_coord(current_row, ic->location.column, RIGHT);
            text_coord.x += TEXT_PADDING;

            draw_text(data->renderer, data->clear_sans, data->text_color,
                      p->label, text_coord.x, text_coord.y);
        }
    }
}

void draw_canvas_to_framebuffer(DrawData* data) {
    // Detach the canvas;
    SDL_SetRenderTarget(data->renderer, NULL);
    SDL_SetRenderDrawColor(data->renderer, 0xff, 0x00, 0xff, 0xff);
    SDL_RenderClear(data->renderer);

    char buffer[256];
    sprintf(buffer, "FPS: %.2f", 1.0 / data->dt);
    draw_text(data->renderer, data->clear_sans, data->white_color, buffer, 0, 0);

    SDL_Rect dest_rect;
    SDL_Rect origin_rect;
    SDL_Rect* dest = NULL;
    SDL_Rect* origin = NULL;
    if(!data->zoomed_in) {
        dest_rect.h = data->height;
        dest_rect.w = width_preserve_ratio(data->height);
        dest_rect.y = 0;
        dest_rect.x = (data->width - dest_rect.w) / 2;
        dest = &dest_rect;
    } else {
        Vec2* zoom_origin = &data->zoom_origin;
        origin_rect.x = zoom_origin->x;
        origin_rect.y = zoom_origin->y;
        origin_rect.h = data->height;
        origin_rect.w = data->width;

        origin = &origin_rect;
    }

    SDL_RenderCopyEx(data->renderer, data->canvas,
                     origin, dest, 0, NULL, SDL_FLIP_NONE);

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
    char* output_filename = "output.bmp";
    char* raster_script = "./raster ";
    char* raster_command = (char*) malloc(strlen(output_filename) +
                                          strlen(raster_script) + 1);
    strcpy(raster_command, raster_script);
    strcat(raster_command, output_filename);

    save_texture(data->renderer, data->canvas, output_filename);

    system(raster_command);
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
