
#ifndef SEARCH_H
#define SEARCH_H

#include "board.h"
#include "transpositiointable.h"

typedef enum {
    MIN_MAX,                // Standard min max
    ALPHA_BETA,             // Standard alpha-beta
    ALHPA_BETA_ORDERED,     // Alpha beta with move ordering
    ITERATIVE_DEEPENING,    // Iterative deepening with alpha beta and move ordering
} SearchAlg;



Move search_best_move(Board* board, AttackTable* attack_table, TTable* t_table, int depth, SearchAlg alg);

#endif