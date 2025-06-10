/**
 * @brief   A module for handling attack tables.
 *
 *          Attack-tables are bitboards storing pseudo-legal moves for each
 *          square and each piece-type. They are intended to be pre-computed
 *          and used as lookup-tables. a1 corresponds to index 0 and h8 to 
 *          index 63.
 * 
 * @file    attacktable.h
 * @author  Algot Heimerson
 * @date    2025-01-20
 * 
 */

#ifndef ATTACKTABLE_H
#define ATTACKTABLE_H

#include "piece.h"
#include <stdint.h>

typedef struct attack_table {
    uint64_t king_table[64];
    uint64_t queen_table[64];
    uint64_t rook_table[64];
    uint64_t bishop_table[64];
    uint64_t knight_table[64];
    uint64_t white_pawn_table[64];
    uint64_t black_pawn_table[64];
    uint64_t white_pawn_attack_table[64];
    uint64_t black_pawn_attack_table[64];
} AttackTable;

AttackTable* attack_table_create();

void attack_table_print(uint64_t bit_board);

void attack_table_destroy(AttackTable* attack_table);

#endif
