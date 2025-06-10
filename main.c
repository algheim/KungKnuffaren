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


int main(void) {
    printf("Enter your move ('a2a4' to move a piece from a2 to a4)\n");
    Board* board = board_create();
    board_set_start(board);
    board_draw(board);

    AttackTable* attack_table = attack_table_create();

    Move* moves = get_legal_moves(board, attack_table);

    int i = 0;
    print_move(moves[0]);
    print_move(moves[1]);
    while (moves[i].from_x != 0 && moves[i].to_x != 0) {
        print_move(moves[i]);
        i++;
    }

    board_destroy(board);
    attack_table_destroy(attack_table);
    free(moves);

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
