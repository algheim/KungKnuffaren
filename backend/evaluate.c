#include "board.h"
#include "piece.h"
#include <stdint.h>
#include <stdio.h>

int get_total_piece_value(uint64_t pieces, Board* board);
int get_mobility_score(uint64_t pieces, Board* board);
int get_piece_value(PieceType type, Board* board);
int get_piece_bonus(PieceType type, int index);
int mirror_index(int index);

#define QUEEN_VALUE     900
#define ROOK_VALUE      500 
#define BISHOP_VALUE    300
#define KNIGHT_VALUE    300
#define PAWN_VALUE      100


static const int WHITE_KNIGHT_BONUS[64] = {
    -50,-40,-30,-30,-30,-30,-40,-50,
    -40,-20,  0,  0,  0,  0,-20,-40,
    -30,  0, 10, 15, 15, 10,  0,-30,
    -30,  5, 15, 20, 20, 15,  5,-30,
    -30,  0, 15, 20, 20, 15,  0,-30,
    -30,  5, 10, 15, 15, 10,  5,-30,
    -40,-20,  0,  5,  5,  0,-20,-40,
    -50,-40,-30,-30,-30,-30,-40,-50,
};

static const int WHITE_PAWN_BONUS[64] = {
    0,  0,  0,  0,  0,  0,  0,  0,
    50, 50, 50, 50, 50, 50, 50, 50,
    10, 10, 20, 30, 30, 20, 10, 10,
    5,  5, 10, 25, 25, 10,  5,  5,
    0,  0,  0, 20, 20,  0,  0,  0,
    5, -5,-10,  0,  0,-10, -5,  5,
    5, 10, 10,-20,-20, 10, 10,  5,
    0,  0,  0,  0,  0,  0,  0,  0
};

static const int WHITE_ROOK_BONUS[64] = {
    0,  0,  0,  0,  0,  0,  0,  0,
    5, 10, 10, 10, 10, 10, 10,  5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    0,  0,  0,  5,  5,  0,  0,  0
};

static const int WHITE_BISHOP_BONUS[64] = {
    -20,-10,-10,-10,-10,-10,-10,-20,
    -10,  0,  0,  0,  0,  0,  0,-10,
    -10,  0,  5, 10, 10,  5,  0,-10,
    -10,  5,  5, 10, 10,  5,  5,-10,
    -10,  0, 10, 10, 10, 10,  0,-10,
    -10, 10, 10, 10, 10, 10, 10,-10,
    -10,  5,  0,  0,  0,  0,  5,-10,
    -20,-10,-10,-10,-10,-10,-10,-20
};

static const int WHITE_QUEEN_BONUS[64] = {
    -20,-10,-10, -5, -5,-10,-10,-20,
    -10,  0,  0,  0,  0,  0,  0,-10,
    -10,  0,  5,  5,  5,  5,  0,-10,
    -5,  0,  5,  5,  5,  5,  0, -5,
    0,  0,  5,  5,  5,  5,  0, -5,
    -10,  5,  5,  5,  5,  5,  0,-10,
    -10,  0,  5,  0,  0,  0,  0,-10,
    -20,-10,-10, -5, -5,-10,-10,-20
};

static const int WHITE_KING_BONUS[64] = {
    -30,-40,-40,-50,-50,-40,-40,-30,
    -30,-40,-40,-50,-50,-40,-40,-30,
    -30,-40,-40,-50,-50,-40,-40,-30,
    -30,-40,-40,-50,-50,-40,-40,-30,
    -20,-30,-30,-40,-40,-30,-30,-20,
    -10,-20,-20,-20,-20,-20,-20,-10,
    20, 20,  0,  0,  0,  0, 20, 20,
    20, 30, 10,  0,  0, 10, 30, 20
};

// Positive value = good for white. Negative value = good for black.
int evaluate_board(Board* board) {
    int white_score = 0;
    int black_score = 0;

    uint64_t white_pieces = board->bit_boards[WHITE_PIECES];
    uint64_t black_pieces = board->bit_boards[BLACK_PIECES];

    white_score += get_total_piece_value(white_pieces, board);
    black_score += get_total_piece_value(black_pieces, board);

    white_score += get_mobility_score(white_pieces, board);
    black_score += get_mobility_score(black_pieces, board);

    return white_score - black_score;
}

int get_total_piece_value(uint64_t pieces, Board* board) {
    int total_value = 0;

    while (pieces) {
        int current_index = __builtin_ctzll(pieces);
        pieces &= pieces - 1;

        PieceType type = board_get_piece(current_index, board);

        total_value += get_piece_value(type, board);
        total_value += get_piece_bonus(type, current_index);
    }   
    
    return total_value;
}


int get_mobility_score(uint64_t pieces, Board* board) {
    return 0;
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


int get_piece_bonus(PieceType type, int index) {
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


int mirror_index(int index) {
    return ((7 - (index / 8)) * 8) + (index % 8);
}
