
#include <stdlib.h>
#include "transpositiointable.h"
#include <stdio.h>


static int nearest_power_of_two(int n);

TTable* tt_create(int size_MB) {
    TTable* t_table = malloc(sizeof(TTable));

    int capacity = (size_MB * 1024ULL * 1024ULL) / sizeof(TTEntry);
    t_table->capacity = nearest_power_of_two(capacity);
    t_table->data = calloc(t_table->capacity, sizeof(TTEntry));
    t_table->entry_count = 0;
    t_table->current_age = 0;

    for (int i = 0 ; i < t_table->capacity ; i++) {
        t_table->data[i].is_active = false;
        t_table->data[i].best_move = move_create(0, 0, 0);
    }

    return t_table;
}

void tt_store(TTable* t_table, uint64_t zobrist_key, int depth, int score, TTEntryType type, Move best_move) {
    // For some reason this is the same as zobrist_key % capacity :o
    int index = zobrist_key & (t_table->capacity - 1);
    
    if (t_table->data[index].is_active && t_table->data[index].depth > depth) {
        return;
    }

    TTEntry new_entry = (TTEntry) {
        .zobrist_key = zobrist_key,
        .entry_type = type,
        .score = score,
        .depth = depth,
        .age = t_table->current_age,
        .best_move = best_move,
        .is_active = true
    };

    t_table->data[index] = new_entry;
}


TTEntry* tt_lookup(TTable* t_table, uint64_t zobrist_key, int* tt_hits, int* tt_lookups) {
    int index = zobrist_key & (t_table->capacity - 1);
    TTEntry* entry = &(t_table->data[index]);
    (*tt_lookups)++;

    if (entry->is_active && entry->zobrist_key == zobrist_key) {
        (*tt_hits)++;
        return entry;
    }

    return NULL;
}


void tt_destroy(TTable* t_table) {
    free(t_table->data);
    free(t_table);
}


static int nearest_power_of_two(int n) {
    int p = 1;
    while (p * 2 <= n) {
        p *= 2;
    }
    return p;
}