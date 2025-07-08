#include "search.h"
#include <stdio.h>
#include <time.h>

static int positions_searched = 0;
static double elapsed_time = -1;
static Move best_move_found = -1;

#define LARGE_POSITIVE 100000
#define LARGE_NEGATIVE -100000

int alpha_beta(Board* board, AttackTable* attack_table, int alpha, int beta, int depth);
int min_max(Board* board, AttackTable* attack_table, int depth);

int search_captures_only(Board* board, AttackTable* attack_table, int alpha, int beta);
void print_search_stats();

Move search_best_move(Board* board, AttackTable* attack_table, int depth) {
    clock_t start_time = clock();
    positions_searched = 0;
    best_move_found = move_create(0, 0, 0);

    int move_count = 0;
    int best_score = LARGE_NEGATIVE;
    int best_index = -1;
    
    Move* legal_moves = board_get_legal_moves(board, attack_table, &move_count);

    for (int i = 0 ; i < move_count ; i++) {
        board_push_move(legal_moves[i], board);
        board_change_turn(board);

        //int score = -min_max(board, attack_table, depth);
        int score = -alpha_beta(board, attack_table, LARGE_NEGATIVE, LARGE_POSITIVE, depth);

        board_change_turn(board);
        board_pop_move(board);
        
        if (score >= best_score) {
            best_score = score;
            best_index = i;
        }
    }

    clock_t end_time = clock();
    elapsed_time = (double)(end_time - start_time) / CLOCKS_PER_SEC;
    best_move_found = legal_moves[best_index];

    print_search_stats();

    return legal_moves[best_index];
}

// Alpha = The minimum score of the current player
// Beta = The maximum score the opponent is willing to tolerate
int alpha_beta(Board* board, AttackTable* attack_table, int alpha, int beta, int depth) {
    positions_searched++;
    if (depth == 0) {
        if (board->turn) {
            return board_evaluate_current(board);
        }
        return - board_evaluate_current(board);
    }

    int move_count = 0;
    Move* legal_moves = board_get_legal_moves(board, attack_table, &move_count);

    if (move_count == 0) {
        return LARGE_NEGATIVE;
    }

    board_change_turn(board);
    for (int i = 0 ; i < move_count ; i++) {
        board_push_move(legal_moves[i], board);

        int score = - alpha_beta(board, attack_table, -beta, -alpha, depth - 1);

        board_pop_move(board);

        if (score >= beta) {
            board_change_turn(board);
            return beta;
        }

        if (score >= alpha) {
            alpha = score;
        }

        if (alpha >= beta) {
            board_change_turn(board);
            return alpha;
        }
    }

    board_change_turn(board);
    return alpha;
}


// Returns the evaluation for the best move according to the current player
int min_max(Board* board, AttackTable* attack_table, int depth) {
    positions_searched++;
    if (depth == 0) {
        if (board->turn) {
            return board_evaluate_current(board);
        }
        return - board_evaluate_current(board);
    }

    int move_count = 0;
    Move* legal_moves = board_get_legal_moves(board, attack_table, &move_count);

    if (move_count == 0) {
        return LARGE_NEGATIVE;
    }

    int best_score = LARGE_NEGATIVE;

    board_change_turn(board);

    for (int i = 0 ; i < move_count ; i++) {
        board_push_move(legal_moves[i], board);

        int score = - min_max(board, attack_table, depth - 1);

        board_pop_move(board);

        if (score > best_score) {
            best_score = score;
        }
    }

    board_change_turn(board);

    return best_score;
}

int search_captures_only(Board* board, AttackTable* attack_table, int alpha, int beta) {
    positions_searched++;
}


void print_search_stats() {
    printf("\n------ Search statistics ------\n");
    printf("Positions searched: \t%d\n", positions_searched);
    printf("Time: \t\t\t%.3f\n", elapsed_time);
    printf("Best move from->to: \t%d->%d\n", move_get_from_index(best_move_found), move_get_to_index(best_move_found));
}
