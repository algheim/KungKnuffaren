#ifndef TRANSPOSITIONTABLE_H
#define TRANSPOSITIONTABLE_H

#include <stdint.h>
#include <stdbool.h>
#include "move.h"

typedef enum {
    ALPHA_ENTRY,
    BETA_ENTRY,
    EXACT_ENTRY,
} TTEntryType;

typedef struct {
    bool is_active;
    uint64_t zobrist_key;
    TTEntryType entry_type;
    int score;
    int depth;
    int age;
    Move best_move;
} TTEntry;

typedef struct {
    int capacity;
    int entry_count;
    int current_age;
    TTEntry* data;
} TTable;

TTable* tt_create(int size_MB);

void tt_store(TTable* t_table, uint64_t zobrist_key, int depth, int score, TTEntryType type, Move best_move);

TTEntry* tt_lookup(TTable* t_table, uint64_t zobrist_key);

void tt_destroy(TTable* t_table);

#endif