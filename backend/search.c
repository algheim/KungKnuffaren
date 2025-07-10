#define LARGE_POSITIVE 100000
#define LARGE_NEGATIVE -100000

#include "search.h"
#include "evaluate.h"
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

static int positions_searched = 0;
static double elapsed_time = -1;
static Move best_move_found = -1;
static SearchAlg current_algorithm = -1;

typedef struct {
    int index;
    int score;
} IndexScore;

int min_max(Board* board, AttackTable* attack_table, int depth);
int alpha_beta(Board* board, AttackTable* attack_table, int alpha, int beta, int depth);
int alpha_beta_ordered(Board* board, AttackTable* attack_table, int alpha, int beta, int depth);

int search_captures_only(Board* board, AttackTable* attack_table, int alpha, int beta, int depth);

// Move ordering
int get_move_score(Board* board, Move move);
int move_compare_function(const void* m1, const void* m2);
IndexScore* get_move_order(Board* board, Move* moves, int move_count);

void print_search_stats();


Move search_best_move(Board* board, AttackTable* attack_table, int depth, SearchAlg alg) {
    clock_t start_time = clock();
    positions_searched = 0;
    best_move_found = move_create(0, 0, 0);
    current_algorithm = alg;

    int move_count = 0;
    int best_score = LARGE_NEGATIVE;
    int best_index = -1;
    
    Move* legal_moves = board_get_legal_moves(board, attack_table, &move_count);
    IndexScore* index_scores = get_move_order(board, legal_moves, move_count);

    for (int i = 0 ; i < move_count ; i++) {
        board_push_move(legal_moves[index_scores[i].index], board);
        board_change_turn(board);
        int score;

        switch(alg) {

            case MIN_MAX:
                score = -min_max(board, attack_table, depth - 1);
                break;
            case ALPHA_BETA:
                score = -alpha_beta(board, attack_table, LARGE_NEGATIVE, LARGE_POSITIVE, depth - 1);
                break;
            case ALHPA_BETA_ORDERED:
                score = -alpha_beta_ordered(board, attack_table, LARGE_NEGATIVE, LARGE_POSITIVE, depth - 1);
                break;
            default:
                fprintf(stderr, "Invalid search algorithm\n");
                exit(1);
                break;
        }

        board_change_turn(board);
        board_pop_move(board);
        
        if (score >= best_score) {
            best_score = score;
            best_index = index_scores[i].index;
        }
    }

    clock_t end_time = clock();
    elapsed_time = (double)(end_time - start_time) / CLOCKS_PER_SEC;
    best_move_found = legal_moves[best_index];

    print_search_stats();

    return legal_moves[best_index];
}

int alpha_beta_ordered(Board* board, AttackTable* attack_table, int alpha, int beta, int depth) {
    if (depth == 0) {
        if (board->turn) {
            return board_evaluate_current(board);
        }
        return - board_evaluate_current(board);
    }
    if (depth == 0) {
        int score = search_captures_only(board, attack_table, alpha, beta, 0);
        return score;
    }
    positions_searched++;

    int move_count = 0;
    Move* legal_moves = board_get_legal_moves(board, attack_table, &move_count);

    if (move_count == 0) {
        return LARGE_NEGATIVE;
    }

    IndexScore* index_scores = get_move_order(board, legal_moves, move_count);

    board_change_turn(board);
    for (int i = 0 ; i < move_count ; i++) {
        board_push_move(legal_moves[index_scores[i].index], board);

        int score = - alpha_beta(board, attack_table, -beta, -alpha, depth - 1);

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

// Alpha = The minimum score of the current player
// Beta = The maximum score the opponent is willing to tolerate
int alpha_beta(Board* board, AttackTable* attack_table, int alpha, int beta, int depth) {
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

        int score = - alpha_beta(board, attack_table, -beta, -alpha, depth - 1);

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
int min_max(Board* board, AttackTable* attack_table, int depth) {
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

int search_captures_only(Board* board, AttackTable* attack_table, int alpha, int beta, int depth) {
    positions_searched++;
    int score = board->turn ? board_evaluate_current(board) : -board_evaluate_current(board);
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
        //board_draw(board);
        //exit(1);
        return score;
    }

    IndexScore* index_scores = get_move_order(board, legal_captures, move_count);
    /*for (int i = 0 ; i < move_count ; i++) {
        printf("i: %d score: %d\n", index_scores[i].index, index_scores[i].score);
        move_print(legal_captures[index_scores[i].index]);
    }

    exit(1);*/

    for (int i = 0 ; i < move_count ; i++) {
        board_push_move(legal_captures[index_scores[i].index], board);
        board_change_turn(board);
        //board_push_move(legal_captures[i], board);

        score = -search_captures_only(board, attack_table, -beta, -alpha, depth + 1);
        board_pop_move(board);
        board_change_turn(board);

        if (score >= beta) {
            return beta;
        }

        if (score > alpha) {
            alpha = score;
        }
    }

    return alpha;
}

int move_compare_function(const void* m1, const void* m2) {
    return -(((IndexScore*)m1)->score - ((IndexScore*)m2)->score);
}

int get_move_score(Board* board, Move move) {
    PieceType from_piece = board_get_piece(move_get_from_index(move), board);
    PieceType to_piece = board_get_piece(move_get_to_index(move), board);
    
    if (to_piece == -1) {
        return 0;
    }

    int captured_value = get_piece_score(to_piece, board);
    int from_value = get_piece_score(from_piece, board);

    return captured_value * 10 - from_value;
}

IndexScore* get_move_order(Board* board, Move* moves, int move_count) {
    IndexScore* index_scores = calloc(move_count, sizeof(IndexScore));

    for (int i = 0 ; i < move_count ; i++) {
        index_scores[i].index = i;
        index_scores[i].score = get_move_score(board, moves[i]);
    }

    qsort(index_scores, move_count, sizeof(IndexScore), move_compare_function);

    return index_scores;
}


void print_search_stats() {
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
    
    default:
        break;
    }

    printf("Positions searched: \t%d\n", positions_searched);
    printf("Time: \t\t\t%.3f\n", elapsed_time);
    printf("Best move from->to: \t%d->%d\n", move_get_from_index(best_move_found), move_get_to_index(best_move_found));
}
