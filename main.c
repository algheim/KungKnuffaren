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

Move read_move();
Move parse_move(char* string);
void print_move(Move move);
uint32_t move_generation_test(Board* board, AttackTable* attack_table, int depth);


int main(void) {
    Board* board = board_create();
    board_set_start(board);
    AttackTable* attack_table = attack_table_create();
    int move_count = 0;
    
    int total_moves = move_generation_test(board, attack_table, 2);
    printf("Total moves: %d\n", total_moves);

    board_destroy(board);
    attack_table_destroy(attack_table);
}

uint32_t move_generation_test(Board* board, AttackTable* attack_table, int depth) {
    if (depth == 0) {
        return 1;
    }
    
    int move_count = 0;
    Move* moves = get_legal_moves(board, attack_table, &move_count);
    uint32_t total_moves = 0;

    for (int i = 0 ; i < move_count ; i++) {
        board_make_move(moves[i], board);
        board_change_turn(board);
        total_moves += move_generation_test(board, attack_table, depth - 1);
        board_unmake_move(moves[i], board);
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


Move read_move() {
    printf("Make a move: ");
    char input_string[5];

    fgets(input_string, 5, stdin);

    printf("%s\n", input_string);

    return parse_move(input_string);
}

Move parse_move(char* string) {
    Move move = {};

    move.from_x = string[0] - 'a';
    move.from_y = 8 - (string[1] - '0');
    move.to_x = string[2] - 'a';
    move.to_y = 8 - (string[3] - '0');

    return move;
} 

void print_move(Move move) {
    printf("------ Move ------ \n");
    printf("from_x = %d\n", move.from_x);
    printf("from_y = %d\n", move.from_y);
    printf("to_x = %d\n", move.to_x);
    printf("to_y = %d\n", move.to_y);
    printf("score: %d\n", move.score);
    printf("------------------ \n");
}
