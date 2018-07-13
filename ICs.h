#ifndef ICS_H
#define ICS_H 1

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

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

typedef enum {
    UP,
    DOWN,
} ICOrientation;

// NOTE(erick): The IC location on the breadboard can be
//  column uniquely defined using three values: column, row and orientation.
// - The column represents in which column the IC is. This value ranges from
//    0 to 3. Values between 1 and 3 are the actual columns of the breadboard.
//    The value 0 is used to represent an IC outside of the breadboard.
// - The row represents in which row of the breadboard the pin 1 of the IC is
//    located. This value ranges from 1 to 64.
// - The orientation tells whether the IC is pointing UP (towards the first row
//    of the breadboard) or DOWN (towards the last row).

typedef struct {
    uint column;
    uint row;
    ICOrientation orientation;
} BreadboardLocation;

typedef enum {
    NON_SPECIAL,
    VCC,
    GND,
    NOT_CONNECTED,
} PinType;

typedef struct {
    PinType type;
    bool goes_outside;
    char* label;
} Pin;

typedef struct {
    char* name;
    char* code;
    Pin* pins;
    uint n_pins;

    BreadboardLocation location;
} IC;

typedef struct {
    IC* data;
    usize count;
    usize capacity;
} ICList;

typedef enum {
    HOVERING,
    SELECTING,
} SelectionState;

typedef struct {
    IC* selected_ic;

    SelectionState state;
    uint row;
    uint column;
} Selection;

bool row_is_inside_ic(IC*, uint);
bool try_to_move_ic(ICList, IC*, int32, int32);
void move_selection(ICList, Selection*, int32, int32);
void try_to_select_ic(ICList, Selection*);
void rotate_ic(IC*);
uint count_outside_ics(ICList);
bool move_outside_ic_in(ICList, uint, uint, uint);
void put_ic_outside(IC*);

#endif
