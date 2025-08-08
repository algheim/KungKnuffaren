
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

typedef struct {
    Board* board;
    AttackTable* attack_table;
    TTable* t_table;
    int alpha;
    int beta;
    int depth;
    int root_depth;
    Move* best_move;
} SearchParams;

Move search_best_move(Board* board, AttackTable* attack_table, int depth, SearchAlg alg);


#endif