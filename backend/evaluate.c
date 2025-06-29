#include "board.h"
#include <stdint.h>

#define QUEEN_VALUE     900
#define ROOK_VALUE      500 
#define BISHOP_VALUE    300
#define KNIGHT_VALUE    300
#define PAWN_VALUE      100

int get_piece_score(uint64_t pieces, Board* board);

// Returns how good the positioin is for white.
int evaluate_board(Board* board) {
    int white_score = 0;
    int black_score = 0;

    uint64_t white_pieces = board->bit_boards[WHITE_PIECES];
    uint64_t black_pieces = board->bit_boards[BLACK_PIECES];

    white_score += get_piece_score(white_pieces, board);
    black_score += get_piece_score(black_pieces, board);

    return white_score - black_score;
}

int get_piece_score(uint64_t pieces, Board* board) {
    int total_value = 0;

    while (pieces) {
        int current_index = __builtin_ctzll(pieces);
        pieces &= pieces - 1;

        PieceType type = board_get_piece(current_index, board);

        switch (type) {
            case WHITE_QUEEN:
            case BLACK_QUEEN:
                total_value += QUEEN_VALUE;
                break;
            
            case WHITE_ROOK:
            case BLACK_ROOK:
                total_value += ROOK_VALUE;
                break;

            case WHITE_BISHOP:
            case BLACK_BISHOP:
                total_value += BISHOP_VALUE;
                break;

            case WHITE_KNIGHT:
            case BLACK_KNIGHT:
                total_value += KNIGHT_VALUE;
                break;

            case WHITE_PAWN:
            case BLACK_PAWN:
                total_value += PAWN_VALUE;
                break;

            default:
                break;
        }
    }   
    
    return total_value;
}
