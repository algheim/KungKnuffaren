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

typedef enum {
    NORMAL_MOVE_FLAG = 0,
    EN_PASSANT_FLAG = 1,
    EN_PASSANT_AVAILABLE_FLAG = 2,
    QUEEN_PROMOTION_FLAG = 3,
    ROOK_PROMOTION_FLAG = 4,
    BISHOP_PROMOTION_FLAG = 5,
    KNIGHT_PROMOTION_FLAG = 6,
    CASTLE_FLAG = 7,
} MoveFlag;

typedef uint16_t Move;

int move_get_from_index(Move move);

MoveFlag move_get_flag(Move move);

int move_get_to_index(Move move);

Move move_create(int from_index, int to_index, int flag);

bool move_exists(Move move);

void move_print(Move move);

#endif