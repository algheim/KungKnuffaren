
#include "movegenerator.h"

Move* get_legal_moves(Board* board, AttackTable* attack_table) {
    if (board->turn) {
        Move* pseudo_legal_moves = get_pseudo_legal_white_moves(board, attack_table);
        return clean_move_array(board, pseudo_legal_moves);
    }
    return get_pseudo_legal_black_moves(board, attack_table);
}