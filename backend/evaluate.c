#include "board.h"
#include "piece.h"
#include <stdint.h>
#include <stdio.h>

float get_total_piece_value(uint64_t pieces, Board* board);
float get_piece_bonus(PieceType type, int index);
int mirror_index(int index);

#define QUEEN_VALUE     900
#define ROOK_VALUE      500 
#define BISHOP_VALUE    300
#define KNIGHT_VALUE    300
#define PAWN_VALUE      100


static const float WHITE_KNIGHT_BONUS[64] = {
    -0.5, -0.4, -0.3, -0.3, -0.3, -0.3, -0.4, -0.5,
    -0.4, -0.2,  0.0,  0.0,  0.0,  0.0, -0.2, -0.4,
    -0.3,  0.0,  0.1,  0.2,  0.2,  0.1,  0.0, -0.3,
    -0.3,  0.1,  0.2,  0.3,  0.3,  0.2,  0.1, -0.3,
    -0.3,  0.0,  0.2,  0.3,  0.3,  0.2,  0.0, -0.3,
    -0.3,  0.1,  0.1,  0.2,  0.2,  0.1,  0.1, -0.3,
    -0.4, -0.2,  0.0,  0.1,  0.1,  0.0, -0.2, -0.4,
    -0.5, -0.4, -0.3, -0.3, -0.3, -0.3, -0.4, -0.5
};

static const float WHITE_PAWN_BONUS[64] = {
     0.0,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0,
     0.5,  0.5,  0.5,  0.5,  0.5,  0.5,  0.5,  0.5,
     0.1,  0.1,  0.2,  0.3,  0.3,  0.2,  0.1,  0.1,
     0.05, 0.05, 0.1,  0.25, 0.25, 0.1,  0.05, 0.05,
     0.0,  0.0,  0.0,  0.2,  0.2,  0.0,  0.0,  0.0,
     0.05,-0.05,-0.1,  0.0,  0.0,-0.1, -0.05, 0.05,
     0.05, 0.1,  0.1, -0.2, -0.2, 0.1,  0.1,  0.05,
     0.0,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0
};

static const float WHITE_ROOK_BONUS[64] = {
     0.0,  0.0,  0.0,  0.05, 0.05, 0.0,  0.0,  0.0,
    -0.05, 0.0,  0.0,  0.0,  0.0,  0.0,  0.0, -0.05,
    -0.05, 0.0,  0.0,  0.0,  0.0,  0.0,  0.0, -0.05,
    -0.05, 0.0,  0.0,  0.0,  0.0,  0.0,  0.0, -0.05,
    -0.05, 0.0,  0.0,  0.0,  0.0,  0.0,  0.0, -0.05,
    -0.05, 0.0,  0.0,  0.0,  0.0,  0.0,  0.0, -0.05,
     0.05, 0.1,  0.1,  0.1,  0.1,  0.1,  0.1,  0.05,
     0.0,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0
};

static const float WHITE_BISHOP_BONUS[64] = {
    -0.2, -0.1, -0.1, -0.1, -0.1, -0.1, -0.1, -0.2,
    -0.1,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0, -0.1,
    -0.1,  0.0,  0.1,  0.2,  0.2,  0.1,  0.0, -0.1,
    -0.1,  0.1,  0.1,  0.2,  0.2,  0.1,  0.1, -0.1,
    -0.1,  0.0,  0.2,  0.2,  0.2,  0.2,  0.0, -0.1,
    -0.1,  0.2,  0.2,  0.2,  0.2,  0.2,  0.2, -0.1,
    -0.1,  0.1,  0.0,  0.0,  0.0,  0.0,  0.1, -0.1,
    -0.2, -0.1, -0.1, -0.1, -0.1, -0.1, -0.1, -0.2
};

static const float WHITE_QUEEN_BONUS[64] = {
    -0.2, -0.1, -0.1, -0.05, -0.05, -0.1, -0.1, -0.2,
    -0.1,  0.0,  0.0,  0.0,   0.0,  0.0,  0.0, -0.1,
    -0.1,  0.0,  0.05, 0.05,  0.05, 0.05, 0.0, -0.1,
    -0.05, 0.0,  0.05, 0.05,  0.05, 0.05, 0.0, -0.05,
     0.0,  0.0,  0.05, 0.05,  0.05, 0.05, 0.0, -0.05,
    -0.1,  0.05, 0.05, 0.05,  0.05, 0.05, 0.0, -0.1,
    -0.1,  0.0,  0.05, 0.0,   0.0,  0.0,  0.0, -0.1,
    -0.2, -0.1, -0.1, -0.05, -0.05, -0.1, -0.1, -0.2
};

static const float WHITE_KING_BONUS[64] = {
    -0.3, -0.4, -0.4, -0.5, -0.5, -0.4, -0.4, -0.3,
    -0.3, -0.4, -0.4, -0.5, -0.5, -0.4, -0.4, -0.3,
    -0.3, -0.4, -0.4, -0.5, -0.5, -0.4, -0.4, -0.3,
    -0.3, -0.4, -0.4, -0.5, -0.5, -0.4, -0.4, -0.3,
    -0.2, -0.3, -0.3, -0.4, -0.4, -0.3, -0.3, -0.2,
    -0.1, -0.2, -0.2, -0.2, -0.2, -0.2, -0.2, -0.1,
     0.2,  0.2,  0.0,  0.0,  0.0,  0.0,  0.2,  0.2,
     0.2,  0.3,  0.1,  0.0,  0.0,  0.1,  0.3,  0.2
};

// Positive value = good for white. Negative value = good for black.
float evaluate_board(Board* board) {
    float white_score = 0;
    float black_score = 0;

    uint64_t white_pieces = board->bit_boards[WHITE_PIECES];
    uint64_t black_pieces = board->bit_boards[BLACK_PIECES];

    white_score += get_total_piece_value(white_pieces, board);
    black_score += get_total_piece_value(black_pieces, board);

    return white_score - black_score;
}

int get_piece_value(PieceType type, Board* board) {
    switch (type) {
        case WHITE_QUEEN:
        case BLACK_QUEEN:
            return QUEEN_VALUE;
            break;
        
        case WHITE_ROOK:
        case BLACK_ROOK:
            return ROOK_VALUE;
            break;

        case WHITE_BISHOP:
        case BLACK_BISHOP:
            return BISHOP_VALUE;
            break;

        case WHITE_KNIGHT:
        case BLACK_KNIGHT:
            return KNIGHT_VALUE;
            break;

        case WHITE_PAWN:
        case BLACK_PAWN:
            return PAWN_VALUE;
            break;

        default:
            return 0;
            break;
    }
}

float get_piece_bonus(PieceType type, int index) {
    if (piece_get_color(type)) {
        index = mirror_index(index);
    }

    switch (type) {
        case WHITE_PAWN:
        case BLACK_PAWN:
            return WHITE_PAWN_BONUS[index];
        case WHITE_KNIGHT:
        case BLACK_KNIGHT:
            return WHITE_KNIGHT_BONUS[index];
        case WHITE_BISHOP:
        case BLACK_BISHOP:
            return WHITE_BISHOP_BONUS[index];
        case WHITE_ROOK:
        case BLACK_ROOK:
            return WHITE_ROOK_BONUS[index];
        case WHITE_QUEEN:
        case BLACK_QUEEN:
            return WHITE_QUEEN_BONUS[index];
        case WHITE_KING:
        case BLACK_KING:
            return WHITE_KING_BONUS[index];

        default:
            return 0;
    }
}

float get_total_piece_value(uint64_t pieces, Board* board) {
    float total_value = 0;

    while (pieces) {
        int current_index = __builtin_ctzll(pieces);
        pieces &= pieces - 1;

        PieceType type = board_get_piece(current_index, board);

        total_value += (float) get_piece_value(type, board);
        total_value += get_piece_bonus(type, current_index);
    }   
    
    return total_value;
}


int mirror_index(int index) {
    return ((7 - (index / 8)) * 8) + (index % 8);
}
