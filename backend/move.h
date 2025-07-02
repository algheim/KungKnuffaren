/**
 * @brief Planning to store moves as just an int in the future.
 * 
 * @file move.h
 * @author Algot Heimerson
 * @date 2025-03-23
 * 
 */

#ifndef MOVE_H
#define MOVE_H

#include <stdint.h>
#include "piece.h"
#include <stdbool.h>

#define EVAL_UNDEFINED 9999999

#define MOVE_CASTLE_NONE 0
#define MOVE_CASTLE_WHITE_KING 1
#define MOVE_CASTLE_WHITE_QUEEN 2
#define MOVE_CASTLE_BLACK_KING 3
#define MOVE_CASTLE_BLACK_QUEEN 4

// xxxxxxxx xxxxxxxx xxxxtttt ttiiiiii

typedef struct move {
    int from_index;
    int to_index;
    int from_type;
    int to_type;
    int initial_score;
    int evaluation_score;
    bool queen_promotion;
    bool rook_promotion;
    bool knigh_promotion;
    bool bishop_promotion;
    char castle;
} Move;

PieceType move_get_from_type(Move move);

PieceType move_get_to_type(Move move);

int move_get_from_index(Move move);

int move_get_to_index(Move move);

Move move_create(int from_index, int to_index, PieceType piece, PieceType capture);

bool move_exists(Move move);

void move_print(Move move);

int move_castle(Move move);

Move move_create_castle(int castle);

#endif