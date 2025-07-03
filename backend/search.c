#include "search.h"

int min_max(Board* board, AttackTable* attack_table, int depth);

Move search_best_move(Board* board, AttackTable* attack_table, int depth) {
    int move_count = 0;
    int best_score = -100000;
    int best_index = -1;
    
    Move* legal_moves = board_get_legal_moves(board, attack_table, &move_count);

    for (int i = 0 ; i < move_count ; i++) {
        board_push_move(legal_moves[i], board);
        board_change_turn(board);

        int score = -min_max(board, attack_table, depth);

        board_change_turn(board);
        board_pop_move(board);
        
        if (score > best_score) {
            best_score = score;
            best_index = i;
        }
    }

    return legal_moves[best_index];
}


// Returns the evaluation for the best move according to the current player
int min_max(Board* board, AttackTable* attack_table, int depth) {
    if (depth == 0) {
        if (board->turn) {
            return board_evaluate_current(board);
        }
        return - board_evaluate_current(board);
    }
    int move_count = 0;
    Move* legal_moves = board_get_legal_moves(board, attack_table, &move_count);

    int best_score = -100000;

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