#ifndef MOEVGENERATOR_H
#define MOEVGENERATOR_H

#include "attacktable.h"
#include "move.h"
#include "board.h"

/**
 * @brief Get the legal moves based on the board's internal turn.
 * 
 * @param board         The board
 * @param attack_table  The attack table. 
 * @return              Array containing legal moves. Ends with an non-existing move. 
 */
Move* get_legal_moves(Board* board, AttackTable* attack_table, int* move_count, uint64_t* attacked_squares);

Move* get_legal_captures(Board* board, AttackTable* attack_table, int* move_count, uint64_t* attacked_squares);

uint64_t get_king_attackers(Board* board, int king_index, AttackTable* attack_table, uint64_t* all_attacks);


#endif