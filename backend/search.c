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

static int positions_searched = 0;
static double elapsed_time = -1;
static Move best_move_found = -1;
static SearchAlg current_algorithm = -1;
static int global_eval = 0;

int alpha_beta(Board* board, AttackTable* attack_table, int alpha, int beta, int depth, int root_depth, Move* best_move);
int search_captures_only(Board* board, AttackTable* attack_table, int alpha, int beta, int depth);
Move iterative_deepening(Board* board, AttackTable* attack_table, int depth);

// Move ordering
ScoredMove* get_scored_moves(Board* board, Move* moves, int move_count);
int get_move_score(Board* board, Move move);
void order_moves_by_guess(Board* board, ScoredMove* moves, int move_count);
void order_moves_by_eval(Board* board, ScoredMove* scored_moves, int move_count);
int compare_guess_scores(const void* m1, const void* m2);
int compare_evals(const void* m1, const void* m2);

void reset_search_stats(SearchAlg alg);
void print_search_stats();
void print_scored_move(ScoredMove move);


Move search_best_move(Board* board, AttackTable* attack_table, int depth, SearchAlg alg) {
    clock_t start_time = clock();
    reset_search_stats(alg);
    int move_count = 0;

    Move* legal_moves = board_get_legal_moves(board, attack_table, &move_count);
    ScoredMove* scored_moves = get_scored_moves(board, legal_moves, move_count);
    free(legal_moves);
    order_moves_by_guess(board, scored_moves, move_count);

    Move best_move = scored_moves[0].move;

    int score = alpha_beta(board, attack_table, LARGE_NEGATIVE, LARGE_POSITIVE, depth, depth, &best_move);

    clock_t end_time = clock();
    elapsed_time = (double)(end_time - start_time) / CLOCKS_PER_SEC;
    best_move_found = best_move;
    global_eval = board->turn ? score : -score;

    print_search_stats();
    free(scored_moves);

    return best_move_found;
}


int alpha_beta(Board* board, AttackTable* attack_table, int alpha, int beta, int depth, int root_depth, Move* best_move) {
    if (depth == 0) {
        return search_captures_only(board, attack_table, alpha, beta, 0);
    }
    positions_searched++;

    // Generate moves
    int move_count = 0;
    Move* legal_moves = board_get_legal_moves(board, attack_table, &move_count);
    if (move_count == 0) {
        if (root_depth == depth) {
            *best_move = move_create(0, 0, 0);
        }
        free(legal_moves);
        return LARGE_NEGATIVE;
    }
    ScoredMove* scored_moves = get_scored_moves(board, legal_moves, move_count);
    free(legal_moves);

    // At root-level, evaluate the best_move from previous iterative depth first
    Move original_best_move = *best_move;
    if (depth == root_depth) {
        Move dummy_move = move_create(0, 0, 0);
        board_change_turn(board);
        board_push_move(*best_move, board);
        alpha = -alpha_beta(board, attack_table, -beta, -alpha, depth - 1, root_depth, &dummy_move);
        board_change_turn(board);
        board_pop_move(board);
    }

    // Evaluate other moves in guess-order:
    order_moves_by_guess(board, scored_moves, move_count);
    board_change_turn(board);
    for (int i = 0 ; i < move_count ; i++) {
        if (scored_moves[i].move == original_best_move) {
            continue;
        }

        board_push_move(scored_moves[i].move, board);
        Move temp_move = move_create(0, 0, 0);
        int score = -alpha_beta(board, attack_table, -beta, -alpha, depth - 1, root_depth, &temp_move);
        board_pop_move(board);

        if (score >= beta) {
            // This should not happen unless something goes wrong..
            if (depth == root_depth) {
                *best_move = scored_moves[i].move;
            }
            board_change_turn(board);
            return beta;
        }

        if (score > alpha) {
            alpha = score;
            if (depth == root_depth) {
                *best_move = scored_moves[i].move;
            }
        }
    }

    free(scored_moves);
    board_change_turn(board);
    return alpha;
}


int search_captures_only(Board* board, AttackTable* attack_table, int alpha, int beta, int depth) {
    positions_searched++;
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
    order_moves_by_guess(board, scored_moves, move_count);

    for (int i = 0 ; i < move_count ; i++) {
        board_push_move(scored_moves[i].move, board);
        board_change_turn(board);

        score = -search_captures_only(board, attack_table, -beta, -alpha, depth + 1);
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

void order_moves_by_guess(Board* board, ScoredMove* scored_moves, int move_count) {
    for (int i = 0 ; i < move_count ; i++) {
        scored_moves[i].guess_score = get_move_score(board, scored_moves[i].move);
    }

    qsort(scored_moves, move_count, sizeof(ScoredMove), compare_guess_scores);
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
