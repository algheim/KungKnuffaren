#include "board.h"
#include "movegenerator.h"
#include <stdlib.h>
#include <stdio.h>
#include "fenparser.h"
#include "evaluate.h"
#include "search.h"
#include <assert.h>

#define WHITE_CASTLE_QUEEN (1 << 0) // 0001
#define WHITE_CASTLE_KING  (1 << 1) // 0010
#define BLACK_CASTLE_QUEEN (1 << 2) // 0100
#define BLACK_CASTLE_KING  (1 << 3) // 1000

#define UNDO_STACK_START_CAPACITY 30


/* -------------------- Internal function declarations --------------------- */
void print_piece(PieceType piece_type);
void make_castle_move(Move move, Board* board);
void unmake_castle_move(Move move, Board* board);
void undo_stack_push(Move move, Board* board);
UndoNode undo_stack_pop(Board* board);
/* -------------------------- External functions --------------------------- */

Board* board_create() {
    Board* board = malloc(sizeof(Board));

    for (int i = 0 ; i < BIT_BOARD_COUNT ; i++) {
        board->bit_boards[i] = 0ULL;
    }
    board->turn = true;
    board->en_passant_index = -1;
    board->castling_rights = 0x0F;
    board->undo_stack = calloc(UNDO_STACK_START_CAPACITY, sizeof(UndoNode));
    board->undo_stack_capacity = UNDO_STACK_START_CAPACITY;
    board->undo_stack_size = 0;

    return board;
}

Board* board_from_fen(char* fen, int size) {
    return fen_to_board(fen, size);
}

char* board_get_fen(Board* board) {
    return board_to_fen(board);
}

int board_evaluate_current(Board* board) {
    return evaluate_board(board);
}

Move board_get_best_move(Board* board, AttackTable* attack_table, int depth) {
    return search_best_move(board, attack_table, depth);
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

void board_push_move(Move move, Board* board) {
    undo_stack_push(move, board);
    int from_index = move_get_from_index(move);
    int to_index = move_get_to_index(move);
    bool turn = board_get_turn(board);
    PieceType piece_type = board_get_piece(from_index, board);

    switch(piece_type) {
        case WHITE_KING:
            board->castling_rights &= ~(WHITE_CASTLE_KING | WHITE_CASTLE_QUEEN);
            break;

        case BLACK_KING:
            board->castling_rights &= ~(BLACK_CASTLE_KING | BLACK_CASTLE_QUEEN);
            break;

        case WHITE_ROOK:
            if (from_index == 0) {
                board->castling_rights &= ~WHITE_CASTLE_QUEEN;
            }
            if (from_index == 7) {
                board->castling_rights &= ~WHITE_CASTLE_KING;
            }
            break;

        case BLACK_ROOK:
            if (from_index == 56) {
                board->castling_rights &= ~BLACK_CASTLE_QUEEN;
            }
            if (from_index == 63) {
                board->castling_rights &= ~BLACK_CASTLE_KING;
            }
            break;

        default:
            break;
    }

    switch (move_get_flag(move)) {
        case CASTLE_FLAG:
            make_castle_move(move, board);
            return;
        case QUEEN_PROMOTION_FLAG:
            board_set_piece(from_index, -1, board);
            board_set_piece(to_index, turn ? WHITE_QUEEN : BLACK_QUEEN, board);
            printf("from, to, type %d %d %d\n", from_index, to_index, turn ? WHITE_QUEEN : BLACK_QUEEN);
            //printf("QUEEN PROMOTED! \n");
            return;
        case ROOK_PROMOTION_FLAG:
            board_set_piece(from_index, -1, board);
            board_set_piece(to_index, turn ? WHITE_ROOK : BLACK_ROOK, board);
            return;
        case BISHOP_PROMOTION_FLAG:
            board_set_piece(from_index, -1, board);
            board_set_piece(to_index, turn ? WHITE_BISHOP : BLACK_BISHOP, board);
            return;
        case KNIGHT_PROMOTION_FLAG:
            board_set_piece(from_index, -1, board);
            board_set_piece(to_index, turn ? WHITE_KNIGHT : BLACK_KNIGHT, board);
            return;
        default:
            board_set_piece(from_index, -1, board);
            board_set_piece(to_index, piece_type, board);
            return;
    }
}

Move board_pop_move(Board* board) {
    if (board->undo_stack_size <= 0) {
        return move_create(0, 0, 0);
    }
    
    UndoNode node = undo_stack_pop(board);
    Move move = node.move;

    board->castling_rights = node.castling_rights;
    board->en_passant_index = node.en_passant_index;
    
    if (move_get_flag(move) == CASTLE_FLAG) {
        unmake_castle_move(move, board);
        return move;
    }
    board_set_piece(move_get_from_index(move), node.move_piece, board);
    board_set_piece(move_get_to_index(move), node.captured_piece, board);
    return move;
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

Move* board_get_legal_moves(Board* board, AttackTable* attack_table, int* move_count) {
    return get_legal_moves(board, attack_table, move_count);
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

void make_castle_move(Move move, Board* board) {
    int to_index = move_get_to_index(move);
    switch (to_index) {
        // White king castle
        case 6:
            board_set_piece(4, -1, board);
            board_set_piece(7, -1, board);
            board_set_piece(6, WHITE_KING, board);
            board_set_piece(5, WHITE_ROOK, board);
            break;
        // White queen castle
        case 2:
            board_set_piece(4, -1, board);
            board_set_piece(0, -1, board);
            board_set_piece(2, WHITE_KING, board);
            board_set_piece(3, WHITE_ROOK, board);
            break;
        // Black king castle
        case 62:
            board_set_piece(60, -1, board);
            board_set_piece(63, -1, board);
            board_set_piece(62, BLACK_KING, board);
            board_set_piece(61, BLACK_ROOK, board);
            break;
        // Black queen castle
        case 58:
            board_set_piece(60, -1, board);
            board_set_piece(56, -1, board);
            board_set_piece(58, BLACK_KING, board);
            board_set_piece(59, BLACK_ROOK, board);
            break;

        default:
            break;
    }
}

void unmake_castle_move(Move move, Board* board) {
    int to_index = move_get_to_index(move);
    switch (to_index) {
        // White king castle
        case 6:
            board_set_piece(6, -1, board);
            board_set_piece(5, -1, board);
            board_set_piece(4, WHITE_KING, board);
            board_set_piece(7, WHITE_ROOK, board);
            break;
        // White queen castle
        case 2:
            board_set_piece(2, -1, board);
            board_set_piece(3, -1, board);
            board_set_piece(4, WHITE_KING, board);
            board_set_piece(0, WHITE_ROOK, board);
            break;
        // Black king castle
        case 62:
            board_set_piece(62, -1, board);
            board_set_piece(61, -1, board);
            board_set_piece(60, BLACK_KING, board);
            board_set_piece(63, BLACK_ROOK, board);
            break;
        // Black queen castle
        case 58:
            board_set_piece(58, -1, board);
            board_set_piece(59, -1, board);
            board_set_piece(60, BLACK_KING, board);
            board_set_piece(56, BLACK_ROOK, board);
            break;
    }
}


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

void undo_stack_push(Move move, Board* board) {
    UndoNode new_node;
    new_node.captured_piece = (int8_t) board_get_piece(move_get_to_index(move), board);
    new_node.move_piece = (int8_t) board_get_piece(move_get_from_index(move), board);
    new_node.move = move;
    new_node.castling_rights = board->castling_rights;
    new_node.en_passant_index = 0;

    if (board->undo_stack_size == board->undo_stack_capacity) {
        board->undo_stack_capacity *= 2;
        board->undo_stack = realloc(board->undo_stack, board->undo_stack_capacity * sizeof(UndoNode));
    }

    board->undo_stack[(board->undo_stack_size)++] = new_node;
}

UndoNode undo_stack_pop(Board* board) {
    if (board->undo_stack_size == 0) {
        fprintf(stderr, "Undo stack is being popped empty!\n");
        exit(1);
    }

    return board->undo_stack[--(board->undo_stack_size)];
}

