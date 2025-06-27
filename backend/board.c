#include "board.h"
#include "movegenerator.h"
#include <stdlib.h>
#include <stdio.h>
#include "fenparser.h"

#define WHITE_CASTLE_QUEEN (1 << 0) // 0001
#define WHITE_CASTLE_KING  (1 << 1) // 0010
#define BLACK_CASTLE_QUEEN (1 << 2) // 0100
#define BLACK_CASTLE_KING  (1 << 3) // 1000

/* -------------------- Internal function declarations --------------------- */
void print_piece(PieceType piece_type);

/* -------------------------- External functions --------------------------- */

Board* board_create() {
    Board* board = malloc(sizeof(Board));

    for (int i = 0 ; i < BIT_BOARD_COUNT ; i++) {
        board->bit_boards[i] = 0ULL;
    }
    board->turn = true;
    board->en_pessant_board = 0ULL;
    board->castling_rights = 0x0F;

    return board;
}

Board* board_from_fen(char* fen, int size) {
    return fen_to_board(fen, size);
}

char* board_get_fen(Board* board) {
    return board_to_fen(board);
}

bool board_get_turn(Board* board) {
    return board->turn;
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

void board_make_move(Move move, Board* board) {
    PieceType piece_type = board_get_piece(move.from_index, board);
    switch(piece_type) {
        case WHITE_KING:
            board->castling_rights &= ~(WHITE_CASTLE_KING | WHITE_CASTLE_QUEEN);
            break;

        case BLACK_KING:
            board->castling_rights &= ~(BLACK_CASTLE_KING | BLACK_CASTLE_QUEEN);
            break;

        case WHITE_ROOK:
            if (move.from_index == 0) {
                board->castling_rights &= ~WHITE_CASTLE_QUEEN;
            }
            if (move.from_index == 7) {
                board->castling_rights &= ~WHITE_CASTLE_KING;
            }
            break;

        case BLACK_ROOK:
            if (move.from_index == 56) {
                board->castling_rights &= ~BLACK_CASTLE_QUEEN;
            }
            if (move.from_index == 63) {
                board->castling_rights &= ~BLACK_CASTLE_KING;
            }
            break;

        default:
            break;
    }

    board_set_piece(move.to_index, piece_type, board);
    board_set_piece(move.from_index, -1, board);
}

void board_unmake_move(Move move, Board* board) {
    PieceType piece_type = board_get_piece(move.to_index, board);
    board_set_piece(move.from_index, piece_type, board);
    board_set_piece(move.to_index, move.to_type, board);
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


bool board_get_piece_color(int index, Board* board) {
    return board->bit_boards[WHITE_PIECES] & (1ULL << index);
}


PieceType get_white_piece_type(Board* board, int index) {
    if (board->bit_boards[WHITE_PAWN] & 1ULL << index) return WHITE_PAWN;
    if (board->bit_boards[WHITE_KNIGHT] & 1ULL << index) return WHITE_KNIGHT;
    if (board->bit_boards[WHITE_BISHOP] & 1ULL << index) return WHITE_BISHOP;
    if (board->bit_boards[WHITE_ROOK] & 1ULL << index) return WHITE_ROOK;
    if (board->bit_boards[WHITE_QUEEN] & 1ULL << index) return WHITE_QUEEN;
    if (board->bit_boards[WHITE_KING] & 1ULL << index) return WHITE_KING;
    return -1;
}


PieceType get_black_piece_type(Board* board, int index) {
    if (board->bit_boards[BLACK_PAWN] & 1ULL << index) return BLACK_PAWN;
    if (board->bit_boards[BLACK_KNIGHT] & 1ULL << index) return BLACK_KNIGHT;
    if (board->bit_boards[BLACK_BISHOP] & 1ULL << index) return BLACK_BISHOP;
    if (board->bit_boards[BLACK_ROOK] & 1ULL << index) return BLACK_ROOK;
    if (board->bit_boards[BLACK_QUEEN] & 1ULL << index) return BLACK_QUEEN;
    if (board->bit_boards[BLACK_KING] & 1ULL << index) return BLACK_KING;
    return -1;
}


void board_set_piece(int index, PieceType new_type, Board* board) {
    PieceType current_type = board_get_piece(index, board);
    if (board_get_piece_color(index, board)) {
        board->bit_boards[WHITE_PIECES] &= ~(1ULL << index);
    }
    else {
        board->bit_boards[BLACK_PIECES] &= ~(1ULL << index);
    }

    if (current_type != -1) {
        board->bit_boards[current_type] &= ~(1ULL << index);
    }
    if (new_type == -1) {
        return;
    }

    board->bit_boards[new_type] |= (1ULL << index);
    // Possibly unnecessary to keep updated all the time.
    if (new_type >= WHITE_KING && new_type <= WHITE_PAWN) {
        board->bit_boards[WHITE_PIECES] |= (1ULL << index);
    }
    else if (new_type >= BLACK_KING && new_type <= BLACK_PAWN) {
        board->bit_boards[BLACK_PIECES] |= (1ULL << index);
    }
}

void board_get_legal_moves(Board* board, AttackTable* attack_table, int* move_count) {
    get_legal_moves(board, attack_table, move_count);
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

void board_set_turn(bool turn, Board* board) {
    board->turn = turn;
}

bool board_white_can_castle_king(Board* board) {
    return board->castling_rights & WHITE_CASTLE_KING;
}

bool board_white_can_castle_queen(Board* board) {
    return board->castling_rights & WHITE_CASTLE_QUEEN;
}

bool board_black_can_castle_king(Board* board) {
    return board->castling_rights & BLACK_CASTLE_KING;
}

bool board_black_can_castle_queen(Board* board) {
    return board->castling_rights & BLACK_CASTLE_QUEEN;
}

void board_set_castling_rights(char side, bool value, Board* board) {
    switch(side) {
        case 'Q':
            if (value) {
                board->castling_rights |= WHITE_CASTLE_QUEEN;
            }
            else {
                board->castling_rights &= ~WHITE_CASTLE_QUEEN;
            }
            break;
        case 'K':
            if (value) {
                board->castling_rights |= WHITE_CASTLE_KING;
            }
            else {
                board->castling_rights &= ~WHITE_CASTLE_KING;
            }
            break;
        case 'q':
            if (value) {
                board->castling_rights |= BLACK_CASTLE_QUEEN;
            }
            else {
                board->castling_rights &= ~BLACK_CASTLE_QUEEN;
            }
            break;
        case 'k':
            if (value) {
                board->castling_rights |= BLACK_CASTLE_KING;
            }
            else {
                board->castling_rights &= ~BLACK_CASTLE_KING;
            }
            break;
        default:
            break;
    }
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

