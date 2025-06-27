#include "board.h"
#include "string.h"
#include <stdio.h>


// Internal declarations
int map_fen_index(int board_index);
int parse_piece_positions(char* fen, int size, Board* board, int* current_index);
int parse_active_color(char* fen, int size, Board* board, int* current_index);

Board* fen_to_board(char* fen, int size) {
    Board* board = board_create();
    int current_index = 0;

    parse_piece_positions(fen, size, board, &current_index);
    parse_active_color(fen, size, board, &current_index);

    return board;
}

int parse_active_color(char* fen, int size, Board* board, int* current_index) {
    if (fen[*current_index] == 'w') {
        board_set_turn(true, board);
    }
    else if (fen[*current_index] == 'b') {
        board_set_turn(false, board);
    }
    else {
        return -1;
    }

    if (++(*current_index) >= size) {
        return -1;
    }
    return 0;
}

int map_fen_index(int board_index) {
    board_index = 63  - board_index;
    int file = board_index % 8;
    int rank = board_index / 8;

    return (rank * 8) + (7 - file);
}

// Returns -1 on failure.
int parse_piece_positions(char* fen, int size, Board* board, int* current_index) {
    int board_index = 0;
    for (int i = 0 ; i < 8 ; i++) {
        while (fen[*current_index] != '/' && fen[*current_index] != ' ') {
            switch(fen[*current_index]) {
                case 'K':
                    board_set_piece(map_fen_index(board_index), WHITE_KING, board);
                    break;
                case 'Q':
                    board_set_piece(map_fen_index(board_index), WHITE_QUEEN, board);
                    break;
                case 'R':
                    board_set_piece(map_fen_index(board_index), WHITE_ROOK, board);
                    break;
                case 'B':
                    board_set_piece(map_fen_index(board_index), WHITE_BISHOP, board);
                    break;
                case 'N':
                    board_set_piece(map_fen_index(board_index), WHITE_KNIGHT, board);
                    break;
                case 'P':
                    board_set_piece(map_fen_index(board_index), WHITE_PAWN, board);
                    break;
                case 'k':
                    board_set_piece(map_fen_index(board_index), BLACK_KING, board);
                    break;
                case 'q':
                    board_set_piece(map_fen_index(board_index), BLACK_QUEEN, board);
                    break;
                case 'r':
                    board_set_piece(map_fen_index(board_index), BLACK_ROOK, board);
                    break;
                case 'b':
                    board_set_piece(map_fen_index(board_index), BLACK_BISHOP, board);
                    break;
                case 'n':
                    board_set_piece(map_fen_index(board_index), BLACK_KNIGHT, board);
                    break;
                case 'p':
                    board_set_piece(map_fen_index(board_index), BLACK_PAWN, board);
                    break;
                default: {
                    int n = fen[*current_index] - '0';
                    board_index += (n - 1);
                    break;
                }
            }
            board_index++;
            (*current_index)++;
            if (*current_index >= size) {
                return -1;
            }
        }

        // Step to after / or ' '
        (*current_index)++;
        if (*current_index >= size)
        {
            return -1;
        }
    }

    return 0;
}
