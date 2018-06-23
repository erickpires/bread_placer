#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

#define sizeof_array(array) (sizeof(array)/sizeof(array[0]))

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
    uint n_pins;
    char* name;
    char* code;
    Pin* pins;
} IC;

typedef struct {
    IC* data;
    usize count;
    usize capacity;
} ICList;

ICList new_ICList() {
    ICList result;

    result.capacity = 16;
    result.data = (IC*) malloc(result.capacity * sizeof(IC));
    result.count = 0;

    return result;
}

void add_to_ic_list(ICList* list, IC ic) {
    if(list->count == list->capacity) {
        list->capacity *= 2;
        list->data = realloc(list->data, list->capacity * sizeof(IC));
    }

    list->data[list->count] = ic;
    list->count++;
}

bool string_begins_with(char* string, char* with) {
    return (strncmp(string, with, strlen(with)) == 0);
}

char* string_after_first_space(char* string) {
    char* result = string;

    while(*result && *result != ' ') { result++; }
    if(*result == ' ') { result++; }

    return result;
}

void report_ic_error(IC* ic) {
    fprintf(stderr, "*Error on IC*\n");
    fprintf(stderr, "\t # pins: %d\n", ic->n_pins);

    if(ic->name) {
        fprintf(stderr, "\t Name: %s\n", ic->name);
    }

    if(ic->code) {
        fprintf(stderr, "\t Code: %s\n", ic->code);
    }
}

void validate_ic(IC ic) {
    bool valid = true;
    for(uint i = 0; i < ic.n_pins; i++) {
        if(!ic.pins[i].label) {
            // If this is our first error.
            if(valid) { report_ic_error(&ic); }

            fprintf(stderr, "Pin_%02d was not assigned\n", i + 1);
            valid = false;
        }
    }

    if(!valid) { exit(2); }
}

void trim_end(char* string) {
    uint len = strlen(string);
    while(len) {
        char c = string[--len];
        if(c == '\n' || c == '\t' || c == ' ') {
            string[len] = '\0';

        } else {break; }
    }
}

char* cpystr(char* string) {
    char* result = (char*) malloc(strlen(string) + 1);
    strcpy(result, string);

    return result;
}

PinType pin_type(char* label) {
    if(string_begins_with(label, "GND"))  { return GND; }
    if(string_begins_with(label, "VCC"))  { return VCC; }
    if(string_begins_with(label, "N.C.")) {return NOT_CONNECTED; }

    return NON_SPECIAL;
}

void assign_pin(IC* ic, uint pin_number, char* label) {
    if(!label) {
        report_ic_error(ic);
        fprintf(stderr, "Null reference for pin [%d] label\n", pin_number);
        exit(3);
    }

    if(strlen(label) == 0) {
        report_ic_error(ic);
        fprintf(stderr, "Zero length label for pin [%d]\n", pin_number);
        exit(3);
    }

    if(pin_number > ic->n_pins) {
        report_ic_error(ic);
        fprintf(stderr, "The IC has only (%d) pins. Pin [%d] is out-of-range\n",
                ic->n_pins, pin_number);
        exit(3);
    }

    uint index = pin_number - 1;
    if(ic->pins[index].label != NULL) {
        report_ic_error(ic);
        fprintf(stderr, "Pin [%d] is already assigned\n", pin_number);
        exit(3);
    }

    Pin current_pin;
    current_pin.label = label;
    current_pin.type = pin_type(label);
    ic->pins[index] = current_pin;
}

int main(int args_count, char** args_values) {
    FILE* input_file = stdin;
    if(args_count > 1) {
        input_file = fopen(args_values[1], "r");
        if(!input_file) {
            fprintf(stderr, "Could not open file: [%s].\n", args_values[1]);
            exit(1);
        }
    }

    ICList ic_list = new_ICList();

    char __line[256];
    bool is_reading_ic = false;
    bool is_reading_pins = false;

    IC current_ic;
    while(!feof(input_file) && fgets(__line, sizeof_array(__line), input_file)) {
        char* line = __line;
        trim_end(line);

        if(!is_reading_ic) {
            if(string_begins_with(line, "IC")) {
                is_reading_ic = true;

                // BUG(erick): Check if there is a number!!!
                current_ic.n_pins = atoi(string_after_first_space(line));
                current_ic.pins = (Pin*) calloc(current_ic.n_pins, sizeof(Pin));
                current_ic.name = NULL;
                current_ic.code = NULL;
            }
        } else {
            if(strlen(line) == 0) {
                is_reading_ic = false;
                is_reading_pins = false;

                validate_ic(current_ic);
                add_to_ic_list(&ic_list, current_ic);
                continue;
            }

            if(!is_reading_pins) {
                if(string_begins_with(line, "Name")) {
                    current_ic.name = cpystr(string_after_first_space(line));
                }

                if(string_begins_with(line, "Code")) {
                    current_ic.code = cpystr(string_after_first_space(line));
                }

                if(string_begins_with(line, "Pins")) {
                    is_reading_pins = true;
                }
            } else {
                uint current_pin;
                char* current_label;

                if(*line != '#') {
                    fprintf(stderr, "Parser is reading pins."
                            " Lines must begin with '#'\n");
                    exit(2);
                }

                line++;
                int read = sscanf(line, "%d ", &current_pin);
                current_label = cpystr(string_after_first_space(line));

                if(!read) {
                    fprintf(stderr, "No number found at: [#%s]\n", line);
                }

                assign_pin(&current_ic, current_pin, current_label);
            }
        }
    }

    // Finishing last IC.
    if(is_reading_ic) {
        is_reading_ic = false;
        is_reading_pins = false;

        validate_ic(current_ic);
        add_to_ic_list(&ic_list, current_ic);
    }

    return 0;
}
