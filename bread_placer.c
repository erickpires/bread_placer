#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>


#include "bread_placer.h"

// TODO(erick): Error codes.

//
// Globals
//
static const char* prj_extension = ".icprj";
static const char* ics_extension = ".ics_list";

//
// Macros
//
#define sizeof_array(array) (sizeof(array)/sizeof(array[0]))

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

ICList parse_ic_list_file(FILE* input_file) {
    ICList ic_list = new_ICList();

    // FIXME(erick): This should not be fixed.
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

    return ic_list;
}

void save_project_file(char* project_filename, ICList* breadboard) {
    FILE* prj_file = fopen(project_filename, "w");
    if(!prj_file) {
        fprintf(stderr, "Could not open project file [%s] to write the project data.\n",
               project_filename);
        exit(4);
    }

    for(uint ic_index = 0; ic_index < breadboard->count; ic_index++) {
        IC* ic = breadboard->data + ic_index;
        BreadboardLocation location = ic->location;

        fprintf(prj_file, "%d: {%d, %d, %d}\n", ic_index, location.column,
                location.row, location.orientation);
    }

    fclose(prj_file);
}

void read_project_file(char* project_filename, ICList* breadboard) {
    FILE* prj_file = fopen(project_filename, "r");
    if(!prj_file) {
        fprintf(stderr, "Could not open project file [%s] to read the project data.\n",
               project_filename);
        exit(4);
    }

    // FIXME(erick): This should not be fixed.
    uint line_number = 0;
    char __line[256];
    while(!feof(prj_file) && fgets(__line, sizeof_array(__line), prj_file)) {
        char* line = __line;
        trim_end(line);
        line_number++;

        uint ic_index;
        uint column;
        uint row;
        uint orientation;

        int read = sscanf(line, "%d: {%d, %d, %d}", &ic_index, &column,
                          &row, &orientation);
        if(read != 4) {
            fprintf(stderr, "Invalid line in project file. Ignoring.\n\t%d: %s\n",
                    line_number, line);
            continue;
        }

        if(ic_index >= breadboard->count) {
            fprintf(stderr, "Invalid IC index [%d] at line (%d) of project file.\n",
                    ic_index, line_number);
            exit(4);
        }

        IC* ic = breadboard->data + ic_index;
        BreadboardLocation* location = &ic->location;

        location->column = column;
        location->row = row;
        location->orientation = orientation;
    }

    fclose(prj_file);
}

char* extension(char* filename) {
    char* result = filename + strlen(filename) - 1;

    while(result != filename) {
        if(*result == '.') { break; }

        result--;
    }

    return result;
}

char* str_n_alloc_cpy(char* str, usize len) {
    char* result = (char*) malloc(len + 1);
    strncpy(result, str, len);
    result[len] = '\0';

    return result;
}

int main(int args_count, char** args_values) {
    if(args_count != 2) {
        fprintf(stderr, "Usage: %s (ics__list_file | prj_file)\n", args_values[0]);
        exit(1);
    }

    char* project_name;
    char* ics_list_filename;
    char* project_filename;

    char* input_filename = args_values[1];
    char* input_extension = extension(input_filename);
    usize input_extension_len = strlen(input_extension);

    bool should_read_prj_file;

    if(input_extension == input_filename) {
        fprintf(stderr, "The input file must have an extension\n");
        exit(2);
    }

    project_name = str_n_alloc_cpy(input_filename,
                                   strlen(input_filename) -
                                   input_extension_len);
    usize project_name_len = strlen(project_name);

    // NOTE(erick): A project file was passed.
    if(strcmp(input_extension, prj_extension) == 0) {
        project_filename = input_filename;
        should_read_prj_file = true;

        usize ics_extension_len = strlen(ics_extension);
        ics_list_filename = (char*) malloc(project_name_len +
                                           ics_extension_len + 1);

        strcpy(ics_list_filename, project_name);
        strcat(ics_list_filename, ics_extension);

    // NOTE(erick): A ics_list file was passed.
    } else if(strcmp(input_extension, ics_extension) == 0) {
        ics_list_filename = input_filename;
        should_read_prj_file = false;

        usize prj_extension_len = strlen(prj_extension);
        project_filename = (char*) malloc(prj_extension_len +
                                                project_name_len + 1);

        strcpy(project_filename, project_name);
        strcat(project_filename, prj_extension);
    } else {
        fprintf(stderr, "You must pass either a project file or a ics_list file\n");
        exit(2);
    }

    FILE* ics_list_file = fopen(ics_list_filename, "r");
    if(!ics_list_file) {
        fprintf(stderr, "Could not open ics_list file [%s] to read the ics data.\n",
                ics_list_filename);
        exit(3);
    }

    ICList ic_list = parse_ic_list_file(ics_list_file);
    fclose(ics_list_file);

    if(should_read_prj_file) {
        read_project_file(project_filename, &ic_list);
    }

    save_project_file(project_filename, &ic_list);

    return 0;
}
