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
#include "search.h"
#include <time.h>

Move read_move();
Move parse_move(char* string);
void print_move(Move move);
uint32_t move_generation_test(Board* board, AttackTable* attack_table, int depth);
void print_moves(Move* moves, int move_count);

void run_uci();
void print_uci_move(Move move);
void uci_parse_pos(Board* board, AttackTable* attack_table, char* current_line);


int main(int argc, char* argv[]) {
    if (argc == 1) {
        run_uci();
        exit(0);
    }
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


void run_uci() {
    char* start_fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    Board* board = board_from_fen(start_fen, strlen(start_fen));
    AttackTable* attack_table = attack_table_create();
    zobrist_init();
    char current_line[4096];

    while (fgets(current_line, sizeof(current_line), stdin)) {
        current_line[strcspn(current_line, "\n")] = 0;

        if (strcmp(current_line, "uci") == 0) {
            printf("id Kungknuffaren\n");
            printf("id Algot Heimerson\n");
            printf("uciok\n");
            fflush(stdout);
        }
        else if (strcmp(current_line, "isready") == 0) {
            printf("readyok\n");
            fflush(stdout);
        }
        else if (strncmp(current_line, "position", 8) == 0) {
            board_destroy(board);
            board = board_from_fen(start_fen, strlen(start_fen));
            uci_parse_pos(board, attack_table, current_line);
            board_draw(board);
        }
        else if (strncmp(current_line, "go", 2) == 0) {
            Move best_move = search_best_move(board, attack_table, 6, 0);
            print_uci_move(best_move);
            fflush(stdout);
        }
        else if (strcmp(current_line, "quit") == 0) {
            break;
        }
    }
    board_destroy(board);
    attack_table_destroy(attack_table);
}

void uci_parse_pos(Board* board, AttackTable* attack_table, char* current_line) {
    if (strncmp(current_line, "position startpos", 17) == 0 &&
        strstr(current_line, "moves") == NULL) {
        return;
    }

    char line_copy[4096];
    strncpy(line_copy, current_line, sizeof(line_copy));
    line_copy[sizeof(line_copy)-1] = '\0';

    char* current_move_str = strtok(&line_copy[23], " ");

    while (current_move_str) {
        int move_count = 0;
        printf("Current move string: %s\n", current_move_str);
        Move move = parse_move(current_move_str);
        // Compare to legal moves to get the correct move flag
        uint64_t attacked_squares = 0ULL;
        Move* legal_moves = board_get_legal_moves(board, attack_table, &move_count, &attacked_squares);

        for (int i = 0 ; i < move_count ; i++) {
            if (move_comp_from_to(move, legal_moves[i])) {
                move = legal_moves[i];
            }
        }

        board_push_move(move, board);
        board_change_turn(board);
        current_move_str = strtok(NULL, " ");
        free(legal_moves);
    }
}

Move parse_move(char* move_str) {
    int from_x = move_str[0] - 'a';
    int from_y = move_str[1] - '0' - 1;
    int to_x = move_str[2] - 'a';
    int to_y = move_str[3] - '0' - 1;
    int from_index = from_y * 8 + from_x;
    int to_index = to_y * 8 + to_x;

    return move_create(from_index, to_index, NORMAL_MOVE_FLAG);
}

void print_uci_move(Move move) {
    int from_rank = move_get_from_index(move) / 8 + 1;
    char from_file = move_get_from_index(move) % 8 + 'a';
    int to_rank = move_get_to_index(move) / 8 + 1;
    char to_file = move_get_to_index(move) % 8 + 'a';

    printf("bestmove %c%d%c%d\n", from_file, from_rank, to_file, to_rank);
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

