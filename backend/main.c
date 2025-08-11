/*
 * @brief The main file of kungknuffaren
 * 
 * @file main.c
 * @author Algot Heimerson
 * @date 2025-01-20
 * 
 */

#include <stdio.h>
#include "board.h"
#include "move.h"
#include "movegenerator.h"
#include <stdlib.h>
#include <string.h>
#include "bitboard.h"
#include "evaluate.h"
#include <time.h>

Move read_move();
Move parse_move(char* string);
void print_move(Move move);
uint32_t move_generation_test(Board* board, AttackTable* attack_table, int depth);
void print_moves(Move* moves, int move_count);


int main(void) {
    AttackTable* attack_table = attack_table_create();
    char* fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    //char* fen = "r4k1r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/5Q1p/PPPBBPPP/RN2K2R w QK - 0 0";

    Board* board = board_from_fen(fen, strlen(fen));

    double start_time = clock();
    uint32_t count = move_generation_test(board, attack_table, 5);
    double end_time = clock();
    double elapsed_time = (double)(end_time - start_time) / CLOCKS_PER_SEC;

    printf("Count: %d\n", count);
    printf("Time: %.3f\n", elapsed_time);
    printf("Nodes per second: %.f\n", count/elapsed_time);

    board_destroy(board);
    attack_table_destroy(attack_table);
}

void print_moves(Move* moves, int move_count) {
    for (int i = 0 ; i < move_count ; i++) {
        printf("from: %d \tto: %d\n", move_get_from_index(moves[i]), move_get_to_index(moves[i]));
    }
}

uint32_t move_generation_test(Board* board, AttackTable* attack_table, int depth) {
    if (depth == 0) {
        return 1;
    }
    
    int move_count = 0;
    uint64_t attacked_squares = 0ULL;
    Move* moves = get_legal_moves(board, attack_table, &move_count, &attacked_squares);
    uint32_t total_moves = 0;

    for (int i = 0 ; i < move_count ; i++) {
        board_push_move(moves[i], board);
        board_change_turn(board);
        total_moves += move_generation_test(board, attack_table, depth - 1);
        board_pop_move(board);
        board_change_turn(board);
    }

    free(moves);
    return total_moves;
}

void print_legal_moves(Move* moves) {
    int i = 0;
    while (move_exists(moves[i])) {
        move_print(moves[i]);
        i++;
    }
}

