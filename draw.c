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

typedef enum {
    ALIGN_LEFT,
    ALIGN_RIGHT,
} Alignmnent;

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

    result.vcc_color.r = 0xff;
    result.vcc_color.b = 0x00;
    result.vcc_color.g = 0x00;
    result.vcc_color.a = 0xff;

    result.gnd_color.r = 0x00;
    result.gnd_color.b = 0x00;
    result.gnd_color.g = 0xff;
    result.gnd_color.a = 0xff;

    result.clear_sans = TTF_OpenFont("ClearSans-Regular.ttf", TEXT_FONT_SIZE);
    result.clear_sans_bold = TTF_OpenFont("ClearSans-Bold.ttf", TEXT_FONT_SIZE);
    result.outside_font = TTF_OpenFont("ClearSans-Regular.ttf", OUTSIDE_TEXT_SIZE);

    return result;
}

static SDL_Texture* text_to_texture(SDL_Renderer* renderer, TTF_Font* font,
                                    SDL_Color color, char* text) {
    SDL_Surface* text_surf = TTF_RenderText_Blended(font, text, color);
    SDL_Texture* result = SDL_CreateTextureFromSurface(renderer, text_surf);

    SDL_FreeSurface(text_surf);
    return result;
}

static void draw_text(SDL_Renderer* renderer, TTF_Font* font, SDL_Color color,
                      char* text, int x0, int y0, int padding_x, int padding_y,
                      Alignmnent alignment) {
    SDL_Texture* text_texture = text_to_texture(renderer, font, color, text);

    int w, h;
    SDL_QueryTexture(text_texture, NULL, NULL, &w, &h);
    SDL_Rect dest_rect;
    dest_rect.x = alignment == ALIGN_LEFT ? x0 + padding_x : x0 - w - padding_x;

    dest_rect.y = y0 + padding_y;
    dest_rect.w = w;
    dest_rect.h = h;

    SDL_RenderCopy(renderer, text_texture, NULL, &dest_rect);

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
    int current_y = 0; //-5;
    for(uint i = 1; i <= 64; i++, current_y += VERTICAL_STRIDE) {
        sprintf(buffer, "%2d", i);

        int current_x = NUMBER_CELL_WIDTH; //5;
        for(uint j = 0; j < 4; j++, current_x += horizontal_stride) {
            draw_text(data->renderer, data->clear_sans, data->text_color, buffer,
                      current_x, current_y, 5, -5, RIGHT);
        }
    }
}

static Vec2 ic_cell_coord(uint row, uint column) {
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
        result.x += NUMBER_CELL_WIDTH + TEXT_CELL_WIDTH;
        break;
    default:
        fprintf(stderr, "Invalid IC column (%d).\n", column);
        exit(5);
    }

    return result;
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
        return loc.row - (ic_height - 1);
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

static void draw_ic_pins(IC* ic, DrawData* data) {
    uint current_row = first_ic_row(ic);
    for(uint pin = 1; pin <= ic->n_pins / 2; pin++, current_row++) {
        uint no_rotation_pin = pin_number_no_rotation(ic, pin);
        uint pin_index = no_rotation_pin - 1;
        Pin* p = ic->pins + pin_index;

        Vec2 text_coord = text_cell_coord(current_row, ic->location.column, LEFT);
        SDL_Color color = data->text_color;
        if(p->type == VCC) { color = data->vcc_color; }
        if(p->type == GND) { color = data->gnd_color; }

        TTF_Font* font = p->goes_outside ?
            data->clear_sans_bold : data->clear_sans;

        draw_text(data->renderer, font, color,
                  p->label, text_coord.x + TEXT_CELL_WIDTH,
                  text_coord.y, TEXT_PADDING, 0, ALIGN_RIGHT);
    }

    current_row--;
    for(uint pin = ic->n_pins / 2 + 1; pin <= ic->n_pins; pin++, current_row--) {
        uint no_rotation_pin = pin_number_no_rotation(ic, pin);
        uint pin_index = no_rotation_pin - 1;
        Pin* p = ic->pins + pin_index;

        Vec2 text_coord = text_cell_coord(current_row,
                                          ic->location.column, RIGHT);
        SDL_Color color = data->text_color;
        if(p->type == VCC) { color = data->vcc_color; }
        if(p->type == GND) { color = data->gnd_color; }

        TTF_Font* font = p->goes_outside ?
            data->clear_sans_bold : data->clear_sans;

        draw_text(data->renderer, font, color,
                  p->label, text_coord.x, text_coord.y, TEXT_PADDING, 0,
                  ALIGN_LEFT);
    }
}

static void draw_ic_name(IC* ic, DrawData* data, SDL_Rect ic_rect) {
    SDL_Texture* name_text = text_to_texture(data->renderer, data->clear_sans_bold,
                                             data->text_color, ic->name);
    SDL_Texture* code_text = text_to_texture(data->renderer, data->clear_sans,
                                             data->text_color, ic->code);

    SDL_Point center = {.x = ic_rect.x + ic_rect.w / 2,
                        .y = ic_rect.y + ic_rect.h / 2};

    int name_w, name_h;
    SDL_QueryTexture(name_text, NULL, NULL, &name_w, &name_h);
    SDL_Rect name_rect = {.x = center.x - name_w / 2,
                          .w = name_w, .h = name_h};

    int code_w, code_h;
    SDL_QueryTexture(code_text, NULL, NULL, &code_w, &code_h);
    SDL_Rect code_rect = {.x = center.x - code_w / 2,
                          .w = code_w, .h = code_h};

    double rotation;
    if(ic->location.orientation == UP) {
        name_rect.y = center.y - name_h - TEXT_PADDING;
        code_rect.y = center.y + TEXT_PADDING;
        rotation = 0.0;
    } else {
        name_rect.y = center.y + TEXT_PADDING;
        code_rect.y = center.y - name_h - TEXT_PADDING;
        rotation = 180.0;
    }

    SDL_RenderCopyEx(data->renderer, name_text, NULL, &name_rect, rotation, NULL,
                     SDL_FLIP_NONE);
    SDL_RenderCopyEx(data->renderer, code_text, NULL, &code_rect, rotation, NULL,
                     SDL_FLIP_NONE);

    SDL_DestroyTexture(name_text);
    SDL_DestroyTexture(code_text);
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

        draw_ic_name(ic, data, ic_outside);
        draw_ic_pins(ic, data);
    }
}

void draw_selection(DrawData* data, Selection selection) {
    Vec2 origin = ic_cell_coord(selection.row, selection.column);

    SDL_Rect selection_rect = {.x = origin.x,
                               .y = origin.y,
                               .h = VERTICAL_STRIDE,
                               .w = IC_CELL_WIDTH};

    if(selection.state == HOVERING) {
        SDL_SetRenderDrawColor(data->renderer, 0x00, 0x00, 0xbb, 0xff);
    } else {
        SDL_SetRenderDrawColor(data->renderer, 0xaa, 0x00, 0xbb, 0xff);
    }

    SDL_RenderFillRect(data->renderer, &selection_rect);
}

void draw_outside_ics_count(DrawData* data, ICList ic_list) {
    uint count = count_outside_ics(ic_list);

    char buffer[256];
    sprintf(buffer, "Outside: %d", count);

    SDL_Texture* text_texture = text_to_texture(data->renderer, data->outside_font,
                                                data->white_color, buffer);
    int text_h, text_w;
    SDL_QueryTexture(text_texture, NULL, NULL, &text_w, &text_h);

    SDL_Rect text_rect = {.y = TEXT_PADDING, .w = text_w, .h = text_h};
    text_rect.x = data->width - text_rect.w - TEXT_PADDING;

    SDL_Rect bg_rect = {.x = text_rect.x - 1 * TEXT_PADDING,
                        .y = text_rect.y - 1 * TEXT_PADDING,
                        .h = text_rect.h + 2 * TEXT_PADDING,
                        .w = text_rect.w + 2 * TEXT_PADDING};

    SDL_SetRenderDrawColor(data->renderer, 0x33, 0x33, 0x33, 0xff);
    SDL_RenderFillRect(data->renderer, &bg_rect);
    SDL_RenderCopy(data->renderer, text_texture, NULL, &text_rect);

    SDL_DestroyTexture(text_texture);
}

void draw_outside_ics_list(DrawData* data, ICList ic_list, uint selected) {
    // ISSUE(erick): This should be dynamic allocated.
    char buffer[4096];
    char* write_ptr = buffer;

    for(usize ic_index = 0; ic_index < ic_list.count; ic_index++) {
        IC* ic = ic_list.data + ic_index;
        if(ic->location.column == 0) {
            char tmp[16];

            strcpy(write_ptr, ic->name);
            write_ptr += strlen(ic->name);

            sprintf(tmp, " (%d)\n", ic->n_pins);
            strcpy(write_ptr, tmp);
            write_ptr += strlen(tmp);
        }
    }

    SDL_Surface* text_surf = TTF_RenderText_Blended_Wrapped(data->outside_font, buffer,
                                                            data->white_color,
                                                            data->width);
    SDL_Texture* text_texture = SDL_CreateTextureFromSurface(data->renderer,
                                                             text_surf);
    SDL_FreeSurface(text_surf);

    int text_h, text_w;
    SDL_QueryTexture(text_texture, NULL, NULL, &text_w, &text_h);

    SDL_Rect text_rect = {.x = TEXT_PADDING, .y = TEXT_PADDING,
                          .w = text_w, .h = text_h};

    SDL_Rect bg_rect = {.x = text_rect.x - 1 * TEXT_PADDING,
                        .y = text_rect.y - 1 * TEXT_PADDING,
                        .h = text_rect.h + 2 * TEXT_PADDING,
                        .w = text_rect.w + 2 * TEXT_PADDING};

    uint text_vertical_stride = text_h / count_outside_ics(ic_list);
    SDL_Rect selected_bg = {.x = bg_rect.x,
                            .y = selected * text_vertical_stride + TEXT_PADDING,
                            .w = bg_rect.w,
                            .h = text_vertical_stride};

    SDL_SetRenderDrawColor(data->renderer, 0x33, 0x33, 0x33, 0xff);
    SDL_RenderFillRect(data->renderer, &bg_rect);

    SDL_SetRenderDrawColor(data->renderer, 0x11, 0x99, 0x11, 0xff);
    SDL_RenderFillRect(data->renderer, &selected_bg);

    SDL_RenderCopy(data->renderer, text_texture, NULL, &text_rect);

    SDL_DestroyTexture(text_texture);
}

void draw_saving_screen(DrawData* data) {
    char* msg = "Saving...";
    SDL_Surface* text_surf = TTF_RenderText_Blended(data->clear_sans_bold, msg,
                                                    data->white_color);
    SDL_Texture* text_texture = SDL_CreateTextureFromSurface(data->renderer,
                                                             text_surf);

    int text_h, text_w;
    SDL_QueryTexture(text_texture, NULL, NULL, &text_w, &text_h);

    SDL_Rect text_rect = {.x = data->width / 2 - text_w / 2,
                          .y = data->height / 2 - text_h / 2,
                          .w = text_w, .h = text_h};

    SDL_SetRenderDrawColor(data->renderer, 0x33, 0x33, 0x33, 0xff);
    SDL_RenderClear(data->renderer);
    SDL_RenderCopy(data->renderer, text_texture, NULL, &text_rect);

    SDL_DestroyTexture(text_texture);
}

void draw_canvas_to_framebuffer(DrawData* data) {
    // Detach the canvas;
    SDL_SetRenderTarget(data->renderer, NULL);
    SDL_SetRenderDrawColor(data->renderer, 0x00, 0x00, 0x00, 0xff);
    SDL_RenderClear(data->renderer);

    char buffer[256];
    sprintf(buffer, "FPS: %.2f", 1.0 / data->dt);
    draw_text(data->renderer, data->clear_sans, data->white_color, buffer,
              0, 0, 0, 0, ALIGN_LEFT);

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

void save_image(DrawData* data, char* output_filename) {
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
