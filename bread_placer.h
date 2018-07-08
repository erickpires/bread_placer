#ifndef BREAD_PLACER_H
#define BREAD_PLACER_H 1

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

ICList new_ICList();
void add_to_ic_list(ICList*, IC);
bool string_begins_with(char*, char*);
char* string_after_first_space(char*);
void report_ic_error(IC*);
void validate_ic(IC);
void trim_end(char*);
char* cpystr(char*);
PinType pin_type(char*);
void assign_pin(IC*, uint, char*);
ICList parse_ic_list_file(FILE*);

#endif
