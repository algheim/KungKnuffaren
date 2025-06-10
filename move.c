
#include "move.h"


Move move_create(int from_index, int to_index, PieceType piece, PieceType capture) {
    Move move = (Move) {
        .from_index = from_index,
        .to_index = to_index,
        .from_type = piece,
        .to_type = capture
    };

    move.score = -1;

    return move;
}

bool move_exists(Move move) {
    return move.from_index != -1;
}