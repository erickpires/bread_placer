#include "ICs.h"

bool row_is_inside_ic(IC* ic, uint row) {
    uint min_row, max_row;

    if(ic->location.orientation == UP) {
        min_row = ic->location.row;
        max_row = min_row + (ic->n_pins / 2 - 1);
    } else {
        max_row = ic->location.row;
        min_row = max_row - (ic->n_pins / 2 - 1);
    }

    return (row >= min_row && row <= max_row);
}

bool try_to_move_ic(ICList list, IC* ic, int32 d_column, int32 d_row) {
    uint min_row, max_row;

    if(ic->location.orientation == UP) {
        min_row = ic->location.row;
        max_row = min_row + (ic->n_pins / 2 - 1);
    } else {
        max_row = ic->location.row;
        min_row = max_row - (ic->n_pins / 2 - 1);
    }

    uint new_column = ic->location.column + d_column;
    uint new_min_row = min_row + d_row;
    uint new_max_row = max_row + d_row;

    if(new_column < 1) { return false; }
    if(new_column > 3) { return false; }

    if(new_min_row < 1)  { return false; }
    if(new_max_row > 64) { return false; }

    for(usize ic_index = 0; ic_index < list.count; ic_index++) {
        IC* test_ic = list.data + ic_index;

        if(test_ic->location.column != new_column) { continue; }
        if(test_ic == ic) { continue; }

        // Testing collisions
        if(row_is_inside_ic(test_ic, new_min_row)) { return false; }
        if(row_is_inside_ic(test_ic, new_max_row)) { return false; }
    }

    // NOTE(erick): No collisions. We can move the IC.
    ic->location.column += d_column;
    ic->location.row += d_row;
    return true;
}

void move_selection(ICList list, Selection* selection, int32 d_column, int32 d_row) {
    if(selection->state == SELECTING) {
        bool success = try_to_move_ic(list, selection->selected_ic, d_column, d_row);
        if(!success) {
            return;
        }
    }

    selection->column += d_column;
    if(selection->column > 3) { selection->column = 3; }
    if(selection->column < 1) { selection->column = 1; }

    selection->row += d_row;
    if(selection->row > 64) { selection->row = 64; }
    if(selection->row < 1) { selection->row = 1; }

}

void try_to_select_ic(ICList list, Selection* selection) {
    for(usize ic_index = 0; ic_index < list.count; ic_index++) {
        IC* ic = list.data + ic_index;

        if(ic->location.column != selection->column) { continue; }

        if(row_is_inside_ic(ic, selection->row)) {
            selection->selected_ic = ic;
            selection->state = SELECTING;

            return;
        }
    }
}

void rotate_ic(IC* ic) {
    if(ic->location.orientation == UP) {
        ic->location.orientation = DOWN;
        ic->location.row += (ic->n_pins / 2 - 1);
    } else {
        ic->location.orientation = UP;
        ic->location.row -= (ic->n_pins / 2 - 1);
    }
}

uint count_outside_ics(ICList ic_list) {
    uint count = 0;
    for(usize ic_index = 0; ic_index < ic_list.count; ic_index++) {
        if(ic_list.data[ic_index].location.column == 0) {
            count++;
        }
    }

    return count;
}

bool move_outside_ic_in(ICList ic_list, uint which, uint row, uint column) {
    IC* to_move = NULL;
    for(usize ic_index = 0; ic_index < ic_list.count; ic_index++) {
        IC* current_ic = ic_list.data + ic_index;
        if(current_ic->location.column == 0) {
            if(which == 0) {
                to_move = current_ic;
                break;
            } else { which--; }
        }
    }

    if(!to_move) { return false; }

    to_move->location.row = 0;
    // to_move->location.column = 0; NOTE(erick): Already is zero!!
    to_move->location.orientation = UP;

    return try_to_move_ic(ic_list, to_move, column, row);
}

void put_ic_outside(IC* ic) {
    ic->location.column = 0;
}
