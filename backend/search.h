
#ifndef SEARCH_H
#define SEARCH_H

#include "board.h"

typedef enum {
    min_max_move,
    alpha_beta_modd,
    alpha_beta_ordered_mode,
    alpha_beta_ultimate_mode,
} SearchAlg;

Move search_best_move(Board* board, AttackTable* attack_table, int depth);



#endif