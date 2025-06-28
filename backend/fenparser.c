#include "board.h"
#include "string.h"
#include <stdio.h>
#include <stdlib.h>

#define FEN_BUF_SIZE 88

// Internal declarations
int map_fen_index(int board_index);
int parse_piece_positions(char* fen, int size, Board* board, int* current_index);
int parse_active_color(char* fen, int size, Board* board, int* current_index);
int parse_castling_rights(char* fen, int size, Board* board, int* current_index);
int parse_en_pessent_targets(char* fen, int size, Board* board, int* current_index);

int add_fen_char(char* fen, char c, int* current_index);
int set_piece_chars(char* fen, Board* board, int* current_index);
int set_active_color_char(char* fen, Board* board, int* current_index);
int set_castling_rights(char* fen, Board* board, int* current_index);

Board* fen_to_board(char* fen, int size) {
    Board* board = board_create();
    int current_index = 0;

    if (parse_piece_positions(fen, size, board, &current_index) == -1) {
        return NULL;
    }
    if (parse_active_color(fen, size, board, &current_index) == -1) {
        return NULL;
    }
    if (parse_castling_rights(fen, size, board, &current_index) == -1) {
        return NULL;
    }

    return board;
}

char* board_to_fen(Board* board) {
    char* fen = calloc(FEN_BUF_SIZE, sizeof(char));
    int current_index = 0;

    set_piece_chars(fen, board, &current_index);

    set_active_color_char(fen, board, &current_index);

    set_castling_rights(fen, board, &current_index);

    // En pessent
    add_fen_char(fen, '-', &current_index);
    add_fen_char(fen, ' ', &current_index);

    // Half clock move
    add_fen_char(fen, '0', &current_index);
    add_fen_char(fen, ' ', &current_index);

    // Fullmove counter
    add_fen_char(fen, '0', &current_index);
    fen[current_index] = '\0';

    return fen;
}

int set_castling_rights(char* fen, Board* board, int* current_index) {
    if (board_white_can_castle_queen(board)) {
        if (add_fen_char(fen, 'Q', current_index) == -1) {
            return -1;
        }
    }
    if (board_white_can_castle_king(board)) {
        if (add_fen_char(fen, 'K', current_index) == -1) {
            return -1;
        }
    }
    if (board_black_can_castle_queen(board)) {
        if (add_fen_char(fen, 'q', current_index) == -1) {
            return -1;
        }
    }
    if (board_black_can_castle_king(board)) {
        if (add_fen_char(fen, 'k', current_index) == -1) {
            return -1;
        }
    }

    if (add_fen_char(fen, ' ', current_index)) {
        return -1;
    }

    return 0;
}

int set_active_color_char(char* fen, Board* board, int* current_index) {
    if (board_get_turn(board)) {
        if (add_fen_char(fen, 'w', current_index) == -1) {
            return -1;
        }
    }
    else {
        if (add_fen_char(fen, 'b', current_index) == -1) {
            return -1;
        }
    }

    if (add_fen_char(fen, ' ', current_index) == -1) {
        return -1;
    }

    return 0;
}

int set_piece_chars(char* fen, Board* board, int* current_index) {
    int empty_counter = 0;

    for (int i = 0; i < 64; i++) {
        int fen_mapping = map_fen_index(i);
        PieceType type = board_get_piece(fen_mapping, board);

        if (i % 8 == 0 && i != 0) {
            if (empty_counter != 0) {
                if (add_fen_char(fen, '0' + empty_counter, current_index) == -1)
                    return -1;
                empty_counter = 0;
            }

            if (add_fen_char(fen, '/', current_index) == -1)
                return -1;
        }

        if (type == -1) {
            empty_counter++;
            continue;
        }

        if (empty_counter != 0) {
            if (add_fen_char(fen, '0' + empty_counter, current_index) == -1)
                return -1;
            empty_counter = 0;
        }

        char piece_char = '?';
        switch(type) {
            case WHITE_KING:    piece_char = 'K'; break;
            case WHITE_QUEEN:   piece_char = 'Q'; break;
            case WHITE_ROOK:    piece_char = 'R'; break;
            case WHITE_BISHOP:  piece_char = 'B'; break;
            case WHITE_KNIGHT:  piece_char = 'N'; break;
            case WHITE_PAWN:    piece_char = 'P'; break;
            case BLACK_KING:    piece_char = 'k'; break;
            case BLACK_QUEEN:   piece_char = 'q'; break;
            case BLACK_ROOK:    piece_char = 'r'; break;
            case BLACK_BISHOP:  piece_char = 'b'; break;
            case BLACK_KNIGHT:  piece_char = 'n'; break;
            case BLACK_PAWN:    piece_char = 'p'; break;
            default: return -1;
        }

        if (add_fen_char(fen, piece_char, current_index) == -1)
            return -1;
    }
    // Final flush of empty squares
    if (empty_counter != 0) {
        if (add_fen_char(fen, '0' + empty_counter, current_index) == -1)
            return -1;
    }

    if (add_fen_char(fen, ' ', current_index) == -1)
        return -1;

    return 0;
}


int add_fen_char(char* fen, char c, int* current_index) {
    fen[*current_index] = c;

    if (++(*current_index) >= FEN_BUF_SIZE) {
        return -1;
    }
    return 0;
}

/* ------------------ board fo fen --------------------*/

/* ------------------ fen to board ------------------*/

// Not implemented!!!!
int parse_en_pessent_targets(char* fen, int size, Board* board, int* current_index) {
    while (fen[*current_index] != ' ') {
        if (++(*current_index) >= size) {
            return -1;
        }
    }
    if (++(*current_index) >= size) {
        return -1;
    }

    return 0;
}

int parse_castling_rights(char* fen, int size, Board* board, int* current_index) {
    board_set_castling_rights('Q', false, board);
    board_set_castling_rights('K', false, board);
    board_set_castling_rights('q', false, board);
    board_set_castling_rights('k', false, board);

    while (fen[*current_index] != ' ') {
        board_set_castling_rights(fen[*current_index], true, board);
        if (++(*current_index) >= size) {
            return -1;
        }
    }

    if (++(*current_index) > size) {
        return -1;
    }
    return 0;
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
    for (int i = 0 ; i < 2 ; i++) {
        if (++(*current_index) >= size) {
            return -1;
        }
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
