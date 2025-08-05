#include "search.h"
#include "evaluate.h"
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

#define LARGE_POSITIVE 100000
#define LARGE_NEGATIVE -100000

static int positions_searched = 0;
static double elapsed_time = -1;
static Move best_move_found = -1;
static SearchAlg current_algorithm = -1;
static float global_eval = 0;

typedef struct {
    Move move;
    float eval_score;
    float guess_score;
} ScoredMove;

float min_max(Board* board, AttackTable* attack_table, int depth);
float alpha_beta(Board* board, AttackTable* attack_table, float alpha, float beta, int depth);
float alpha_beta_ordered(Board* board, AttackTable* attack_table, float alpha, float beta, int depth);
ScoredMove* iterative_deepening(Board* board, AttackTable* attack_table, double time);

float search_captures_only(Board* board, AttackTable* attack_table, float alpha, float beta, int depth);

// Move ordering
ScoredMove* get_scored_moves(Board* board, Move* moves, int move_count);
float get_move_score(Board* board, Move move);
void order_moves_by_guess(Board* board, ScoredMove* moves, int move_count);
void order_moves_by_eval(Board* board, ScoredMove* scored_moves, int move_count);

void print_search_stats();


Move search_best_move(Board* board, AttackTable* attack_table, int depth, SearchAlg alg) {
    clock_t start_time = clock();
    positions_searched = 0;
    best_move_found = move_create(0, 0, 0);
    global_eval = 0;
    current_algorithm = alg;

    int move_count = 0;
    float best_score = LARGE_NEGATIVE;
    int best_index = -1;

    if (alg == ITERATIVE_DEEPENING) {
        ScoredMove* scored_moves = iterative_deepening(board, attack_table, 1);
        clock_t end_time = clock();
        best_move_found = scored_moves[0].move;
        elapsed_time = (double)(end_time - start_time) / CLOCKS_PER_SEC;
        global_eval = board->turn ? scored_moves[0].eval_score : -scored_moves[0].eval_score;
        free(scored_moves);
        print_search_stats();
        return best_move_found;
    }

    Move* legal_moves = board_get_legal_moves(board, attack_table, &move_count);
    ScoredMove* scored_moves = get_scored_moves(board, legal_moves, move_count);
    order_moves_by_guess(board, scored_moves, move_count);
    float alpha = LARGE_NEGATIVE;

    for (int i = 0 ; i < move_count ; i++) {
        board_push_move(scored_moves[i].move, board);
        board_change_turn(board);
        float score;

        switch(alg) {

            case MIN_MAX:
                score = -min_max(board, attack_table, depth - 1);
                break;
            case ALPHA_BETA:
                score = -alpha_beta(board, attack_table, alpha, LARGE_POSITIVE, depth - 1);
                break;
            case ALHPA_BETA_ORDERED:
                score = -alpha_beta_ordered(board, attack_table, alpha, LARGE_POSITIVE, depth - 1);
                break;
                
            default:
                fprintf(stderr, "Invalid search algorithm\n");
                exit(1);
                break;
        }
        if (score > alpha) {
            alpha = score;
        }

        board_change_turn(board);
        board_pop_move(board);
        
        if (score >= best_score) {
            best_score = score;
            best_index = i;
        }
    }

    clock_t end_time = clock();
    elapsed_time = (double)(end_time - start_time) / CLOCKS_PER_SEC;
    best_move_found = scored_moves[best_index].move;
    global_eval = board->turn ? best_score : -best_score;

    print_search_stats();
    free(legal_moves);
    free(scored_moves);

    return best_move_found;
}

/*int alpha_beta_test(Board* board, AttackTable* attack_table, int alpha, int beta, int depth, Move* best_move) {
    if (depth == 0) {
        if (board->turn) {
            return board_evaluate_current(board);
        }
        return - board_evaluate_current(board);
    }
    if (depth == 0) {
        return (ScoredMove) {.move = move_create(0, 0, 0),
                             .eval_score = search_captures_only(board, attack_table, alpha, beta, depth)
                            };
    }

    positions_searched++;

    int move_count = 0;
    Move* legal_moves = board_get_legal_moves(board, attack_table, &move_count);

    if (move_count == 0) {
        return (ScoredMove){.move = move_create(0, 0, 0), .eval_score = LARGE_NEGATIVE};
    }

    Move best_move;

    board_change_turn(board);
    for (int i = 0 ; i < move_count ; i++) {
        board_push_move(legal_moves[i], board);

        int score = - alpha_beta(board, attack_table, -beta, -alpha, depth - 1);

        board_pop_move(board);

        if (score >= beta) {
            break;
        }

        if (score >= alpha) {
            best_move = legal_moves[i];
            alpha = score;
        }
    }

    board_change_turn(board);
    return (ScoredMove) {.move = best_move, .eval_score = alpha};
}*/


ScoredMove* iterative_deepening(Board* board, AttackTable* attack_table, double time) {
    int max_depth = 3;
    int current_depth = 1;
    int move_count = 0;
    
    Move* legal_moves = board_get_legal_moves(board, attack_table, &move_count);
    ScoredMove* scored_moves = get_scored_moves(board, legal_moves, move_count);
    free(legal_moves);
    order_moves_by_guess(board, scored_moves, move_count);

    while(current_depth <= max_depth) {
        for (int i = 0 ; i < move_count ; i++) {
            board_push_move(scored_moves[i].move, board);
            board_change_turn(board);
            
            float score = -alpha_beta_ordered(board, attack_table, LARGE_NEGATIVE, LARGE_POSITIVE, current_depth);
            scored_moves[i].eval_score = score;

            board_pop_move(board);
            board_change_turn(board);
        }

        current_depth++;
        order_moves_by_eval(board, scored_moves, move_count);
    }

    return scored_moves;
}


float alpha_beta_ordered(Board* board, AttackTable* attack_table, float alpha, float beta, int depth) {
    /*if (depth == 0) {
        if (board->turn) {
            return board_evaluate_current(board);
        }
        return - board_evaluate_current(board);
    }*/
    if (depth == 0) {
        float score = search_captures_only(board, attack_table, alpha, beta, 0);
        return score;
    }
    positions_searched++;

    int move_count = 0;
    Move* legal_moves = board_get_legal_moves(board, attack_table, &move_count);

    if (move_count == 0) {
        free(legal_moves);
        return LARGE_NEGATIVE;
    }

    ScoredMove* scored_moves = get_scored_moves(board, legal_moves, move_count);
    free(legal_moves);
    order_moves_by_guess(board, scored_moves, move_count);

    board_change_turn(board);
    for (int i = 0 ; i < move_count ; i++) {
        board_push_move(scored_moves[i].move, board);

        float score = - alpha_beta_ordered(board, attack_table, -beta, -alpha, depth - 1);

        board_pop_move(board);

        if (score >= beta) {
            board_change_turn(board);
            return beta;
        }

        if (score >= alpha) {
            alpha = score;
        }
    }

    free(scored_moves);
    board_change_turn(board);
    return alpha;
}

// Alpha = The minimum score of the current player
// Beta = The maximum score the opponent is willing to tolerate
float alpha_beta(Board* board, AttackTable* attack_table, float alpha, float beta, int depth) {
    /*if (depth == 0) {
        if (board->turn) {
            return board_evaluate_current(board);
        }
        return - board_evaluate_current(board);
    }*/
    if (depth == 0) {
        return search_captures_only(board, attack_table, alpha, beta, depth);
    }
    positions_searched++;

    int move_count = 0;
    Move* legal_moves = board_get_legal_moves(board, attack_table, &move_count);

    if (move_count == 0) {
        return LARGE_NEGATIVE;
    }

    board_change_turn(board);
    for (int i = 0 ; i < move_count ; i++) {
        board_push_move(legal_moves[i], board);

        float score = - alpha_beta(board, attack_table, -beta, -alpha, depth - 1);

        board_pop_move(board);

        if (score >= beta) {
            board_change_turn(board);
            return beta;
        }

        if (score >= alpha) {
            alpha = score;
        }
    }

    board_change_turn(board);
    return alpha;
}


// Returns the evaluation for the best move according to the current player
float min_max(Board* board, AttackTable* attack_table, int depth) {
    if (depth == 0) {
        if (board->turn) {
            return board_evaluate_current(board);
        }
        return - board_evaluate_current(board);
    }
    positions_searched++;

    int move_count = 0;
    Move* legal_moves = board_get_legal_moves(board, attack_table, &move_count);

    if (move_count == 0) {
        free(legal_moves);
        return LARGE_NEGATIVE;
    }

    float best_score = LARGE_NEGATIVE;

    board_change_turn(board);

    for (int i = 0 ; i < move_count ; i++) {
        board_push_move(legal_moves[i], board);

        float score = - min_max(board, attack_table, depth - 1);

        board_pop_move(board);

        if (score > best_score) {
            best_score = score;
        }
    }

    board_change_turn(board);

    return best_score;
}

float search_captures_only(Board* board, AttackTable* attack_table, float alpha, float beta, int depth) {
    positions_searched++;
    float score = board_evaluate_current(board);
    score = board->turn ? score : -score;
    //printf("depth: %d\n", depth);
    
    if (score >= beta) {
        return beta;
    }
    if (depth >= 10) {
        return score;
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
        //board_push_move(legal_captures[i], board);

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

float get_move_score(Board* board, Move move) {
    PieceType from_piece = board_get_piece(move_get_from_index(move), board);
    PieceType to_piece = board_get_piece(move_get_to_index(move), board);
    
    if (to_piece == -1) {
        return 0;
    }

    int captured_value = get_piece_value(to_piece, board);
    int from_value = get_piece_value(from_piece, board);

    return (float) captured_value * 10 - (float) from_value;
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
