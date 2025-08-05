
#ifndef EVALUATE_H
#define EVALUATE_H

#include "board.h"

// Returns how good the positioin is for white.
float evaluate_board(Board* board);

int get_piece_value(uint64_t pieces, Board* board);

#endif