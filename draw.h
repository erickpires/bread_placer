#ifndef DRAW_H
#define DRAW_H 1

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include "ICs.h"

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

#define VERTICAL_STRIDE   (CANVAS_HEIGHT / 63)
#define NUMBER_CELL_WIDTH VERTICAL_STRIDE

#define _REMAINING_SPACE  (CANVAS_WIDTH - 4 * NUMBER_CELL_WIDTH)
#define _LARGE_CELL_WIDTH (_REMAINING_SPACE / 9)
#define TEXT_CELL_WIDTH _LARGE_CELL_WIDTH
#define IC_CELL_WIDTH   _LARGE_CELL_WIDTH


#define LINE_WIDTH 4
#define TEXT_FONT_SIZE 40
#define TEXT_PADDING 10

#define width_preserve_ratio(h) ((h * CANVAS_WIDTH) / CANVAS_HEIGHT)

typedef struct {
    union {
        int32 x;
        int32 w;
    };

    union {
        int32 y;
        int32 h;
    };
} Vec2;

// TODO(erick): Abstract the font data to another struct
typedef struct {
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* canvas;
    SDL_Color text_color;
    // BUG(erick): If I put something here the code stops working.

    SDL_Color white_color;
    TTF_Font* clear_sans;
    TTF_Font* clear_sans_bold;

    int width;
    int height;

    bool zoomed_in;
    Vec2 zoom_origin;

    float dt;

    SDL_Color gnd_color;
    SDL_Color vcc_color;
} DrawData;


DrawData init_SDL();

void prepare_canvas(DrawData*);
void draw_grid(DrawData*);
void draw_numbers(DrawData*);
void draw_ics(DrawData*, ICList);

void draw_canvas_to_framebuffer(DrawData* data);
void swap_buffers(DrawData*);

void save_image(DrawData*);

#endif
