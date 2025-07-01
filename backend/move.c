
#include "move.h"
#include <stdio.h>

Move move_create(int from_index, int to_index, PieceType piece, PieceType capture) {
    Move move = (Move) {
        .from_index = from_index,
        .to_index = to_index,
        .from_type = piece,
        .to_type = capture
    };

    move.initial_score = -1;
    move.evaluation_score = -1;

    return move;
}

bool move_exists(Move move) {
    return move.from_index != -1;
}


void move_print(Move move) {
    printf("------ Move ------ \n");
    printf("from_index = %d\n", move.from_index);
    printf("to_index = %d\n", move.to_index);
    printf("------------------ \n");
}