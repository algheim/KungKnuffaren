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

typedef struct move {
    // To be removed:
    int from_x;
    int from_y;
    int to_x;
    int to_y;

    char from_index;
    char to_index;
    int from_type;
    int to_type;
    int score;
} Move;


PieceType move_get_from_type(Move move);

PieceType move_get_to_type(Move move);

PieceType move_get_from_index(Move move);

PieceType move_get_to_index(Move move);

Move move_create(int from_index, int to_index, PieceType piece, PieceType capture);

bool move_exists(Move move);

#endif