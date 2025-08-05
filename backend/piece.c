#include "piece.h"

bool piece_get_color(PieceType type) {
    switch (type) {
    case WHITE_BISHOP:
    case WHITE_KING:
    case WHITE_KNIGHT:
    case WHITE_PAWN:
    case WHITE_QUEEN:
    case WHITE_ROOK:
        return true;

    default:
        return false;
    }
}