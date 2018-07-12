#ifndef BREAD_PLACER_H
#define BREAD_PLACER_H 1

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

ICList new_ICList();
void add_to_ic_list(ICList*, IC);
bool string_begins_with(char*, char*);
char* string_after_first_space(char*);
void report_ic_error(IC*);
void validate_ic(IC);
void trim_end(char*);
char* cpystr(char*);
PinType pin_type(char*);
void assign_pin(IC*, uint, bool, char*);
ICList parse_ic_list_file(FILE*);

#endif
