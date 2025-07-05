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

Move read_move();
Move parse_move(char* string);
void print_move(Move move);
uint32_t move_generation_test(Board* board, AttackTable* attack_table, int depth);
void print_moves(Move* moves, int move_count);


int main(void) {
    AttackTable* attack_table = attack_table_create();
    char* fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

    Board* board = board_from_fen(fen, strlen(fen));

    board_draw(board);

    printf("Castling rights: %d %d\n", board_white_can_castle_king(board), board_white_can_castle_queen(board));

    Move m1 = move_create(5, 22, 0);
    Move m2 = move_create(12, 28, 0);
    Move m3 = move_create(5, 19, 0);
    Move m4 = move_create(4, 6, CASTLE_FLAG);

    board_push_move(m1, board);
    board_draw(board);
    printf("Castling rights: %d %d\n", board_white_can_castle_king(board), board_white_can_castle_queen(board));

    board_push_move(m2, board);
    board_draw(board);
    printf("Castling rights: %d %d\n", board_white_can_castle_king(board), board_white_can_castle_queen(board));

    board_push_move(m3, board);
    board_draw(board);
    printf("Castling rights: %d %d\n", board_white_can_castle_king(board), board_white_can_castle_queen(board));

    board_push_move(m4, board);
    board_draw(board);
    printf("Castling rights: %d %d\n", board_white_can_castle_king(board), board_white_can_castle_queen(board));

    board_push_move(m4, board);
    board_draw(board);
    printf("Castling rights: %d %d\n", board_white_can_castle_king(board), board_white_can_castle_queen(board)); 

    /*uint32_t count = move_generation_test(board, attack_table, 2);
    printf("Count: %d\n", count);*/

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
    Move* moves = get_legal_moves(board, attack_table, &move_count);
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

