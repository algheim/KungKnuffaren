#ifndef BOARD_H
#define BOARD_H

#include <stdbool.h>
#include <stdint.h>
#include "move.h"
#include "piece.h"
#include "attacktable.h"

#define BIT_BOARD_COUNT 14


typedef struct {
    Move move;
    int8_t en_passant_index;
    int8_t move_piece;
    int8_t captured_piece;
    uint8_t castling_rights;
} UndoNode;

// a1 maps to the least significant bit and h8 maps to the most significant bit
typedef struct board {
    uint64_t bit_boards[14];
    int8_t en_passant_index;
    bool turn;
    uint8_t castling_rights;
    UndoNode* undo_stack;
    int undo_stack_size;
    int undo_stack_capacity;
} Board;


Board* board_create();

Board* board_from_fen(char* fen, int size);

char* board_get_fen(Board* board);

int board_evaluate_current(Board* board);

Move board_get_best_move(Board* board, AttackTable* attack_table, int depth);

void board_draw(Board* board);

void board_push_move(Move move, Board* board);

Move board_pop_move(Board* board);

void board_set_piece(int index, PieceType type, Board* board);

PieceType board_get_piece(int index, Board* board);

// True if white, else false.
bool board_get_piece_color(int index, Board* board);

void board_change_turn(Board* board);

bool board_get_turn(Board* board);

void board_set_start(Board* board);

Move* board_get_legal_moves(Board* board, AttackTable* attack_table, int* move_count);

void board_set_turn(bool turn, Board* board);

bool board_white_can_castle_king(Board* board);

bool board_white_can_castle_queen(Board* board);

bool board_black_can_castle_king(Board* board);

bool board_black_can_castle_queen(Board* board);

void board_set_castling_rights(char side, bool value, Board* board);

void board_destroy(Board* board);


#endif
