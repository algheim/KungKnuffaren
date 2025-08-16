
#include "move.h"
#include <stdio.h>
#include <stdbool.h>


MoveFlag move_get_flag(Move move) {
    return (MoveFlag) (move >> 12 & 0xF);
}

int move_get_from_index(Move move) {
    // typecast??
    return (MoveFlag) (move >> 6 & 0x3F);
}

int move_get_to_index(Move move) {
    return (MoveFlag) (move & 0x3F);
}

Move move_create(int from_index, int to_index, int flag) {
    return ((from_index & 0x3F) << 6) | (to_index & 0x3F) | ((flag & 0xF) << 12);
}

bool move_exists(Move move) {
    return move_get_from_index(move) != move_get_to_index(move); 
}

bool move_comp_from_to(Move m1, Move m2) {
    return move_get_from_index(m1) == move_get_from_index(m2) &&
           move_get_to_index(m1) == move_get_to_index(m2);
}

void move_print(Move move) {
    printf("------ Move ------ \n");
    printf("from_index = %d\n", move_get_from_index(move));
    printf("to_index = %d\n", move_get_to_index(move));
    printf("------------------ \n");
}

