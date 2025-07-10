
#ifndef SEARCH_H
#define SEARCH_H

#include "board.h"

typedef enum {
    MIN_MAX,
    ALPHA_BETA,
    ALHPA_BETA_ORDERED,
    ALPHA_BETA_ULTIMATE,
} SearchAlg;

Move search_best_move(Board* board, AttackTable* attack_table, int depth, SearchAlg alg);



#endif