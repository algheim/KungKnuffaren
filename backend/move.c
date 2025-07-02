
#include "move.h"
#include <stdio.h>
#include <stdbool.h>


Move move_create(int from_index, int to_index, PieceType piece, PieceType capture) {
    Move move = (Move) {
        .from_index = from_index,
        .to_index = to_index,
        .from_type = piece,
        .to_type = capture
    };

    move.initial_score = -1;
    move.evaluation_score = -1;
    move.castle = MOVE_CASTLE_NONE;

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

int move_castle(Move move) {
    return move.castle;
}

Move move_create_castle(int castle) {
    Move move;

    switch(castle) {
        case MOVE_CASTLE_WHITE_KING:
            move.from_index = 4;
            move.to_index = 6;
            break;
        case MOVE_CASTLE_WHITE_QUEEN:
            move.from_index = 4;
            move.to_index = 2;
            break;
        case MOVE_CASTLE_BLACK_KING:
            move.from_index = 60;
            move.to_index = 62;
            break;
        case MOVE_CASTLE_BLACK_QUEEN:
            move.from_index = 60;
            move.to_index = 58;
            break;
        default:
            break;
            move.from_index = -1;
    }

    move.castle = castle;
    move.from_type = -1;
    move.to_type = -1;
    move.initial_score = -1;
    move.evaluation_score = -1;
    move.bishop_promotion = false;
    move.rook_promotion = false;
    move.queen_promotion = false;
    move.knigh_promotion = false;

    return move;
}
