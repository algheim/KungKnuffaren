#include "search.h"
#include "evaluate.h"
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

#define LARGE_POSITIVE 100000
#define LARGE_NEGATIVE -100000

typedef struct {
    Move move;
    int eval_score;
    int guess_score;
} ScoredMove;

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

static int positions_searched = 0;
static double elapsed_time = -1;
static Move best_move_found = -1;
static SearchAlg current_algorithm = -1;
static int global_eval = 0;

// Main search
int alpha_beta(SearchParams params);
int search_captures_only(Board* board, AttackTable* attack_table, TTable* t_table, int alpha, int beta, int depth);
Move iterative_deepening(Board* board, AttackTable* attack_table, int depth, TTable* t_table);
int search_with_tt(SearchParams params, int original_alpha, int original_beta);

// Move ordering
ScoredMove* get_scored_moves(Board* board, Move* moves, int move_count);
int get_move_score(Board* board, Move move);
void order_moves_by_guess(Board* board, ScoredMove* scored_moves, int move_count, Move* best_move);
void order_moves_by_eval(Board* board, ScoredMove* scored_moves, int move_count);
int compare_guess_scores(const void* m1, const void* m2);
int compare_evals(const void* m1, const void* m2);

// Search stats
void reset_search_stats(SearchAlg alg);
void print_search_stats();
void print_scored_move(ScoredMove move);


Move search_best_move(Board* board, AttackTable* attack_table, TTable* t_table, int depth, SearchAlg alg) {
    clock_t start_time = clock();
    reset_search_stats(alg);
    int move_count = 0;

    Move* legal_moves = board_get_legal_moves(board, attack_table, &move_count);
    ScoredMove* scored_moves = get_scored_moves(board, legal_moves, move_count);
    free(legal_moves);
    order_moves_by_guess(board, scored_moves, move_count, NULL);

    Move best_move = scored_moves[0].move;

    //int score = alpha_beta(board, attack_table, LARGE_NEGATIVE, LARGE_POSITIVE, depth, depth, &best_move);
    best_move = iterative_deepening(board, attack_table, depth, t_table);

    clock_t end_time = clock();
    elapsed_time = (double)(end_time - start_time) / CLOCKS_PER_SEC;
    best_move_found = best_move;
    //global_eval = board->turn ? score : -score;

    print_search_stats();
    free(scored_moves);

    t_table->current_age++;

    return best_move_found;
}

/*
 * Updates global_eval.
 */
Move iterative_deepening(Board* board, AttackTable* attack_table, int depth, TTable* t_table) {
    int move_count = 0;
    Move* legal_moves = board_get_legal_moves(board, attack_table, &move_count);
    ScoredMove* scored_moves = get_scored_moves(board, legal_moves, move_count);
    free(legal_moves);
    order_moves_by_guess(board, scored_moves, move_count, NULL);

    Move current_best_move = scored_moves[0].move;

    for (int current_depth = 1 ; current_depth <= depth ; current_depth++) {
        SearchParams params = (SearchParams) {
            .board = board,
            .attack_table = attack_table,
            .t_table = t_table,
            .alpha = LARGE_NEGATIVE,
            .beta = LARGE_POSITIVE,
            .depth = current_depth,
            .root_depth = current_depth,
            .best_move = &current_best_move,
        };
        int score = alpha_beta(params);

        global_eval = board->turn ? score : -score;
    }
    
    return current_best_move;
}


/*
 * Input best_move will be the first move evaluated during search. 
 * Output best_move is the best move found by the search. 
 */
int alpha_beta(SearchParams params) {
    if (params.depth == 0) {
        return search_captures_only(params.board, params.attack_table, params.t_table, params.alpha, params.beta, 0);
    }

    positions_searched++;
    int original_alpha = params.alpha;
    int original_beta = params.beta;

    uint64_t current_hash = board_get_zobrist_hash(params.board);
    TTEntry* tt_entry = tt_lookup(params.t_table, current_hash);

    if (tt_entry && tt_entry->depth >= params.depth && tt_entry->age == params.t_table->current_age) {

        if (tt_entry->entry_type == TT_EXACT) {
            return tt_entry->score;
        }
        if (tt_entry->entry_type == TT_UPPER_BOUND && tt_entry->score < params.alpha) {
            return tt_entry->score;
        }
        if (tt_entry->entry_type == TT_LOWER_BOUND && tt_entry->score >= params.beta) {
            return tt_entry->score;
        }
    }
    if (tt_entry && move_exists(tt_entry->best_move) && params.depth != params.root_depth) {
        *(params.best_move) = tt_entry->best_move;
    }

    // Generate moves
    int move_count = 0;
    Move* legal_moves = board_get_legal_moves(params.board, params.attack_table, &move_count);
    if (move_count == 0) {
        if (params.root_depth == params.depth) {
            *(params.best_move) = move_create(0, 0, 0);
        }
        free(legal_moves);
        return LARGE_NEGATIVE;
    }
    ScoredMove* scored_moves = get_scored_moves(params.board, legal_moves, move_count);
    free(legal_moves);

    // Evaluate moves in guess-order with best_move first if it exists:
    order_moves_by_guess(params.board, scored_moves, move_count, params.best_move);
    board_change_turn(params.board);
    for (int i = 0 ; i < move_count ; i++) {
        board_push_move(scored_moves[i].move, params.board);

        SearchParams updated_params = (SearchParams) {
            .board = params.board,
            .attack_table = params.attack_table,
            .t_table = params.t_table,
            .alpha = -params.beta,
            .beta = -params.alpha,
            .depth = params.depth - 1,
            .root_depth = params.root_depth,
            .best_move = NULL
        };

        int score = -alpha_beta(updated_params);
        board_pop_move(params.board);

        if (score >= params.beta) {
            // This should not happen unless something goes wrong..
            if (params.depth == params.root_depth) {
                *(params.best_move) = scored_moves[i].move;
            }
            board_change_turn(params.board);
            return params.beta;
        }

        if (score > params.alpha) {
            params.alpha = score;
            if (params.depth == params.root_depth) {
                *(params.best_move) = scored_moves[i].move;
            }
        }
    }
    free(scored_moves);
    board_change_turn(params.board);

    TTEntryType type;
    if (params.alpha <= original_alpha) {
        // The search didn't find a move better than prev alpha, so no such scores can exist.
        type = TT_UPPER_BOUND;
    }
    else if (params.alpha >= original_beta) {
        // A beta cutoff occurred, so the score is a lower bound of the true score.
        type = TT_LOWER_BOUND;
    }
    else {
        // The search raised alpha without a beta-cutoff, so the score is exact.
        type = TT_EXACT;
    }

    if (params.best_move) {
        tt_store(params.t_table, current_hash, params.depth, params.alpha, type, *(params.best_move));
    }
    tt_store(params.t_table, current_hash, params.depth, params.alpha, type, move_create(0, 0, 0));

    return params.alpha;
}


int search_captures_only(Board* board, AttackTable* attack_table, TTable* t_table, int alpha, int beta, int depth) {
    positions_searched++;
    int original_alpha = alpha;
    int original_beta = beta;

    uint64_t current_hash = board_get_zobrist_hash(board);
    TTEntry* tt_entry = tt_lookup(t_table, current_hash);

    if (tt_entry && tt_entry->age == t_table->current_age) {
        if (tt_entry->entry_type == TT_EXACT) {
            return tt_entry->score;
        }
        if (tt_entry->entry_type == TT_UPPER_BOUND && tt_entry->score < alpha) {
            return tt_entry->score;
        }
        if (tt_entry->entry_type == TT_LOWER_BOUND && tt_entry->score >= beta) {
            return tt_entry->score;
        }
    }

    int score = board_evaluate_current(board);
    score = board->turn ? score : -score;
    
    if (score >= beta) {
        return beta;
    }

    if (score > alpha) {
        alpha = score;
    }

    int move_count = 0;
    Move* legal_captures = board_get_legal_captures(board, attack_table, &move_count);

    if (move_count == 0) {
        return score;
    }

    ScoredMove* scored_moves = get_scored_moves(board, legal_captures, move_count);
    free(legal_captures);
    order_moves_by_guess(board, scored_moves, move_count, NULL);

    for (int i = 0 ; i < move_count ; i++) {
        board_push_move(scored_moves[i].move, board);
        board_change_turn(board);

        score = -search_captures_only(board, attack_table, t_table, -beta, -alpha, depth + 1);
        board_pop_move(board);
        board_change_turn(board);

        if (score >= beta) {
            free(scored_moves);
            return beta;
        }

        if (score > alpha) {
            alpha = score;
        }
    }

    TTEntryType type;
    if (alpha <= original_alpha) {
        // The search didn't find a move better than prev alpha, so no such scores can exist.
        type = TT_UPPER_BOUND;
    }
    else if (alpha >= original_beta) {
        // A beta cutoff occurred, so the score is a lower bound of the true score.
        type = TT_LOWER_BOUND;
    }
    else {
        // The search raised alpha without a beta-cutoff, so the score is exact.
        type = TT_EXACT;
    }
    
    tt_store(t_table, current_hash, -1, alpha, type, move_create(0, 0, 0));

    free(scored_moves);

    return alpha;
}


ScoredMove* get_scored_moves(Board* board, Move* moves, int move_count) {
    ScoredMove* scored_moves = calloc(move_count, sizeof(ScoredMove));

    for (int i = 0 ; i < move_count ; i++) {
        scored_moves[i].move = moves[i];
        scored_moves[i].guess_score = get_move_score(board, scored_moves[i].move);
    }

    return scored_moves;
}

int get_move_score(Board* board, Move move) {
    PieceType from_piece = board_get_piece(move_get_from_index(move), board);
    PieceType to_piece = board_get_piece(move_get_to_index(move), board);
    
    if (to_piece == -1) {
        return 0;
    }

    int captured_value = get_piece_value(to_piece, board);
    int from_value = get_piece_value(from_piece, board);

    return captured_value * 10 - from_value;
}

void order_moves_by_guess(Board* board, ScoredMove* scored_moves, int move_count, Move* best_move) {
    for (int i = 0 ; i < move_count ; i++) {
        scored_moves[i].guess_score = get_move_score(board, scored_moves[i].move);
    }

    if (best_move && move_count > 1) {
        for (int i = 0 ; i < move_count ; i++) {
            if (scored_moves[i].move == *best_move) {
                ScoredMove temp = scored_moves[i];
                scored_moves[i] = scored_moves[0];
                scored_moves[0] = temp;
                break;
            }
        }
        qsort(&(scored_moves[1]), move_count - 1, sizeof(ScoredMove), compare_guess_scores);
    }
    else {
        qsort(scored_moves, move_count, sizeof(ScoredMove), compare_guess_scores);
    }
}

void order_moves_by_eval(Board* board, ScoredMove* scored_moves, int move_count) {
    qsort(scored_moves, move_count, sizeof(ScoredMove), compare_evals);
}

int compare_guess_scores(const void* m1, const void* m2) {
    return -(((ScoredMove*)m1)->guess_score - ((ScoredMove*)m2)->guess_score);
}

int compare_evals(const void* m1, const void* m2) {
    int a = ((ScoredMove*)m1)->eval_score;
    int b = ((ScoredMove*)m2)->eval_score;

    if (a > b) return -1;  // descending order
    if (a < b) return 1;
    return 0;
}

void print_scored_move(ScoredMove move) {
    int from_index = move_get_from_index(move.move);
    int to_index = move_get_to_index(move.move);
    printf("From: %d, To, %d, guess: %d, eval:%d\n", from_index, to_index, move.guess_score, move.eval_score);
}


void reset_search_stats(SearchAlg alg) {
    current_algorithm = alg;
    positions_searched = 0;
    best_move_found = move_create(0, 0, 0);
    global_eval = 0;
}


void print_search_stats() {
    printf("\n");
    switch (current_algorithm)
    {
        case MIN_MAX:
            printf("-------- Min-max statistics --------\n");
            break;
        case ALPHA_BETA:
            printf("------- Alpha-beta statistics -------\n");
            break;
        case ALHPA_BETA_ORDERED:
            printf("--- Alpha-beta ordered statistics ---\n");
            break;
        case ITERATIVE_DEEPENING:
            printf("------- Iterative deepeniing --------\n");
            break;
        
        default:
            break;
    }

    printf("Positions searched: \t%d\n", positions_searched);
    printf("Time: \t\t\t%.3f\n", elapsed_time);
    printf("Positions/s: \t\t%.f\n", positions_searched / elapsed_time);
    printf("Eval: \t\t\t%.3f\n", global_eval / 100.0);
    printf("Best move from->to: \t%d->%d\n", move_get_from_index(best_move_found), move_get_to_index(best_move_found));
}
