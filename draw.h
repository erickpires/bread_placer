#ifndef DRAW_H
#define DRAW_H 1

#include <SDL2/SDL.h>
#include "SDL2/SDL_ttf.h"

typedef intptr_t isize;
typedef int8_t   int8;
typedef int16_t  int16;
typedef int32_t  int32;
typedef int64_t  int64;

typedef size_t       usize;
typedef uint8_t      uint8;
typedef uint16_t     uint16;
typedef uint32_t     uint32;
typedef uint64_t     uint64;
typedef unsigned int uint;

#define CANVAS_WIDTH  2480
#define CANVAS_HEIGHT 3508

#define LINE_WIDTH 4
#define TEXT_FONT_SIZE 40

#define width_preserve_ratio(h) ((h * CANVAS_WIDTH) / CANVAS_HEIGHT)


// TODO(erick): Abstract the font data to another struct
typedef struct {
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* canvas;
    SDL_Color text_color;
    SDL_Color white_color;
    TTF_Font* clear_sans;
    TTF_Font* clear_sans_bold;

    int width;
    int height;

    float dt;
} DrawData;


DrawData init_SDL();

void draw_breadboard(DrawData*);
void swap_buffers(DrawData*);

void save_image(DrawData*);

#endif
