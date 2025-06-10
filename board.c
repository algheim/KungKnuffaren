#include "board.h"
#include "movegenerator.h"
#include <stdlib.h>
#include <stdio.h>



/* -------------------- Internal function declarations --------------------- */
void print_piece(PieceType piece_type);

/* -------------------------- External functions --------------------------- */

Board* board_create() {
    Board* board = malloc(sizeof(Board));

    for (int i = 0 ; i < BIT_BOARD_COUNT ; i++) {
        board->bit_boards[i] = 0;
    }
    board->turn = true;

    board->en_pessant_board = 0;

    return board;
}

void board_draw(Board* board) {
    printf("   a b c d e f g h \n");
    printf("   --------------- \n");

    for (int i = 7 ; i >= 0 ; i--) {
        printf("%c |", 49 + i);
        for (int j = 0 ; j < 8 ; j++) {
            print_piece(board_get_piece(i * 8 + j, board));
            if (j != 7) {
                printf(" ");
            }
        }
        printf("| %c", 49 + i);
        printf("\n");
    }

    printf("   --------------- \n");
    printf("   a b c d e f g h \n");
}

void board_move(Move move, Board* board) {
    PieceType piece_type = board_get_piece(move.from_index, board);
    board_set_piece(move.to_index, piece_type, board);
    board_set_piece(move.from_index, -1, board);
}

PieceType board_get_piece(int index, Board* board) {
    if (board->bit_boards[WHITE_PIECES] & (1ULL << index)) {
        for (int type = WHITE_KING ; type <= WHITE_PAWN ; type++) {
            if (board->bit_boards[type] & (1ULL << index)) {
                return type;
            }
        }
    }
    else if (board->bit_boards[BLACK_PIECES] & (1ULL << index)) {
        for (int type = BLACK_KING ; type <= BLACK_PAWN ; type++) {
            if (board->bit_boards[type] & (1ULL << index)) {
                return type;
            }
        }
    }

    return -1;
}

void board_set_piece(int index, PieceType type, Board* board) {
    board->bit_boards[type] |= (1ULL << index);

    // Possibly unnecessary to keep updated all the time.
    if (type >= WHITE_KING && type <= WHITE_PAWN) {
        board->bit_boards[WHITE_PIECES] |= (1ULL << index);
    }
    else if (type >= BLACK_KING && type <= BLACK_PAWN) {
        board->bit_boards[BLACK_PIECES] |= (1ULL << index);
    }
}

void board_get_legal_moves(Board* board, AttackTable* attack_table) {
    get_legal_moves(board, attack_table);
}

void board_change_turn(Board* board) {
    board->turn = !board->turn;
}

void board_set_start(Board* board) {
    board_set_piece(63, BLACK_ROOK, board);
    board_set_piece(62, BLACK_KNIGHT, board);
    board_set_piece(61, BLACK_BISHOP, board);
    board_set_piece(60, BLACK_QUEEN, board);
    board_set_piece(59, BLACK_KING, board);
    board_set_piece(58, BLACK_BISHOP, board);
    board_set_piece(57, BLACK_KNIGHT, board);
    board_set_piece(56, BLACK_ROOK, board);

    for (int i = 0 ; i < 8 ; i++) {
        board_set_piece(55 - i, BLACK_PAWN, board);
    }

    for (int i = 0 ; i < 8 ; i++) {
        board_set_piece(8 + i, WHITE_PAWN, board);
    }

    board_set_piece(0, WHITE_ROOK, board);
    board_set_piece(1, WHITE_KNIGHT, board);
    board_set_piece(2, WHITE_BISHOP, board);
    board_set_piece(3, WHITE_QUEEN, board);
    board_set_piece(4, WHITE_KING, board);
    board_set_piece(5, WHITE_BISHOP, board);
    board_set_piece(6, WHITE_KNIGHT, board);
    board_set_piece(7, WHITE_ROOK, board);
}

void board_destroy(Board* board) {
    free(board);
}


/* -------------------------- Internal functions --------------------------- */

void print_piece(PieceType piece_type) {
    switch(piece_type) {
        case WHITE_KING:
            printf("K");
            break;
        case WHITE_QUEEN:
            printf("Q");
            break;
        case WHITE_ROOK:
            printf("R");
            break;
        case WHITE_BISHOP:
            printf("B");
            break;
        case WHITE_KNIGHT:
            printf("N");
            break;
        case WHITE_PAWN:
            printf("P");
            break;

        case BLACK_KING:
            printf("k");
            break;
        case BLACK_QUEEN:
            printf("q");
            break;
        case BLACK_ROOK:
            printf("r");
            break;
        case BLACK_BISHOP:
            printf("b");
            break;
        case BLACK_KNIGHT:
            printf("n");
            break;
        case BLACK_PAWN:
            printf("d");
            break;
        
        default:
            printf(".");
            break;
    }
}

