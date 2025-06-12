#ifndef BOARD_H
#define BOARD_H

#include <stdbool.h>
#include <stdint.h>
#include "move.h"
#include "piece.h"
#include "attacktable.h"

#define BIT_BOARD_COUNT 14



// a1 maps to the least significant bit and h8 maps to the most significant bit
typedef struct board {
    uint64_t bit_boards[14];
    uint64_t en_pessant_board;
    bool turn;
} Board;


Board* board_create();

void board_draw(Board* board);

void board_make_move(Move move, Board* board);

void board_unmake_move(Move move, Board* board);

void board_set_piece(int index, PieceType type, Board* board);

PieceType board_get_piece(int index, Board* board);

// True if white, else false.
bool board_get_piece_color(int index, Board* board);

void board_change_turn(Board* board);

void board_set_start(Board* board);

void board_get_legal_moves(Board* board, AttackTable* attack_table, int* move_count);

void board_destroy(Board* board);


#endif
