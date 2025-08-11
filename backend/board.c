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
void update_castling_rights(Move move, PieceType piece_type, Board* board);
void print_piece(PieceType piece_type);
void make_castle_move(Move move, Board* board);
void make_en_passant_move(Move move, PieceType piece_type,  Board* board);
void unmake_castle_move(Move move, Board* board);
void undo_stack_push(Move move, Board* board);
UndoNode undo_stack_pop(Board* board);

uint64_t rand_uint64(uint64_t seed);
uint64_t calculate_zobrist_hash(Board* board);
int piece_to_zobrist_index(PieceType type);

static uint64_t zobrist_table[64][12];
static uint16_t zobrist_castling[16];
static uint64_t zobrist_en_passant_file[8];
static uint64_t zobrist_side_to_move;


/* ---------------------Internal Zobrist hashing functions ------------------*/

/* XOR-shift algorithm */
uint64_t rand_uint64(uint64_t seed) {
    seed ^= seed >> 12;
    seed ^= seed << 25;
    seed ^= seed >> 27;
    seed *= 0x2545F4914F6CDD1DULL;

    return seed;
}

void zobrist_init() {
    uint64_t rand_seed = 0xCAFEBABEDEADBEEF;
    for (int i = 0 ; i < 64 ; i++) {
        for (int j = 0 ; j < 12 ; j++) {
            rand_seed = rand_uint64(rand_seed);
            zobrist_table[i][j] = rand_seed;
        }
    }

    for (int i = 0 ; i < 16 ; i++) {
        rand_seed = rand_uint64(rand_seed);
        zobrist_castling[i] = rand_seed;
    }

    for (int i = 0 ; i < 8 ; i++) {
        rand_seed = rand_uint64(rand_seed);
        zobrist_en_passant_file[i] = rand_seed;
    }

    rand_seed = rand_uint64(rand_seed);
    zobrist_side_to_move = rand_seed;
}

uint64_t board_get_zobrist_hash(Board* board) {
    return board->current_zobrist_hash;
}

uint64_t calculate_zobrist_hash(Board* board) {
    uint64_t zobrist_hash = 0ULL;
    uint64_t all_pieces = board->bit_boards[WHITE_PIECES] | board->bit_boards[BLACK_PIECES];

    while (all_pieces) {
        int current_index = __builtin_ctzll(all_pieces);
        all_pieces &= all_pieces - 1;

        PieceType type = board_get_piece(current_index, board);
        int type_index = piece_to_zobrist_index(type);

        zobrist_hash ^= zobrist_table[current_index][type_index];
    }

    zobrist_hash ^= zobrist_castling[board->castling_rights];

    if (!board->turn) {
        zobrist_hash ^= zobrist_side_to_move;
    }

    if (board->en_passant_index != -1) {
        zobrist_hash ^= zobrist_en_passant_file[board->en_passant_index % 8];
    }

    return zobrist_hash;
}

int piece_to_zobrist_index(PieceType type) {
    switch (type) {
        case WHITE_KING:   return 0;
        case WHITE_QUEEN:  return 1;
        case WHITE_ROOK:   return 2;
        case WHITE_BISHOP: return 3;
        case WHITE_KNIGHT: return 4;
        case WHITE_PAWN:   return 5;
        case BLACK_KING:   return 6;
        case BLACK_QUEEN:  return 7;
        case BLACK_ROOK:   return 8;
        case BLACK_BISHOP: return 9;
        case BLACK_KNIGHT: return 10;
        case BLACK_PAWN:   return 11;
        default:           return -1;
    }
}


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
    board->current_zobrist_hash = calculate_zobrist_hash(board);

    return board;
}

Board* board_from_fen(char* fen, int size) {
    Board* new_board = fen_to_board(fen, size);
    new_board->current_zobrist_hash = calculate_zobrist_hash(new_board);
    return new_board;
}

char* board_get_fen(Board* board) {
    return board_to_fen(board);
}

int board_evaluate_current(Board* board) {
    return evaluate_board(board);
}

Move board_get_best_move(Board* board, AttackTable* attack_table, TTable* t_table, int depth, SearchAlg alg) {
    return search_best_move(board, attack_table, t_table, depth, alg);
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
    if (!move_exists(move)) {
        return;
    }
    
    undo_stack_push(move, board);
    int from_index = move_get_from_index(move);
    int to_index = move_get_to_index(move);
    bool turn = board_get_turn(board);
    PieceType piece_type = board_get_piece(from_index, board);

    // Remove old en passant hash
    if (board->en_passant_index != -1) {
        board->current_zobrist_hash ^= zobrist_en_passant_file[board->en_passant_index % 8];
    }

    if (move_get_flag(move) == EN_PASSANT_AVAILABLE_FLAG) {
        if (board_get_piece_color(from_index, board)) {
            board->en_passant_index = from_index + 8;
            board->current_zobrist_hash ^= zobrist_en_passant_file[board->en_passant_index % 8];
        }
        else {
            board->en_passant_index = from_index - 8;
            board->current_zobrist_hash ^= zobrist_en_passant_file[board->en_passant_index % 8];
        }
    }
    else {
        board->en_passant_index = -1;
    }

    board->current_zobrist_hash ^= zobrist_castling[board->castling_rights];
    update_castling_rights(move, piece_type, board);
    board->current_zobrist_hash ^= zobrist_castling[board->castling_rights];

    switch (move_get_flag(move)) {
        case CASTLE_FLAG:
            make_castle_move(move, board);
            break;
        case EN_PASSANT_FLAG:
            make_en_passant_move(move, piece_type, board);
            break;
        case QUEEN_PROMOTION_FLAG:
            board_set_piece(from_index, -1, board);
            board_set_piece(to_index, turn ? WHITE_QUEEN : BLACK_QUEEN, board);
            break;
        case ROOK_PROMOTION_FLAG:
            board_set_piece(from_index, -1, board);
            board_set_piece(to_index, turn ? WHITE_ROOK : BLACK_ROOK, board);
            break;
        case BISHOP_PROMOTION_FLAG:
            board_set_piece(from_index, -1, board);
            board_set_piece(to_index, turn ? WHITE_BISHOP : BLACK_BISHOP, board);
            break;
        case KNIGHT_PROMOTION_FLAG:
            board_set_piece(from_index, -1, board);
            board_set_piece(to_index, turn ? WHITE_KNIGHT : BLACK_KNIGHT, board);
            break;
        default:
            board_set_piece(from_index, -1, board);
            board_set_piece(to_index, piece_type, board);
            break;
    }
}

Move board_pop_move(Board* board) {
    if (board->undo_stack_size <= 0) {
        return move_create(0, 0, 0);
    }
    
    UndoNode node = undo_stack_pop(board);
    Move move = node.move;

    board->current_zobrist_hash ^= zobrist_castling[board->castling_rights];
    board->castling_rights = node.castling_rights;
    board->current_zobrist_hash ^= zobrist_castling[board->castling_rights];

    if (board->en_passant_index != -1) {
        board->current_zobrist_hash ^= zobrist_en_passant_file[board->en_passant_index % 8];
    }
    board->en_passant_index = node.en_passant_index;
    if (board->en_passant_index != -1) {
        board->current_zobrist_hash ^= zobrist_en_passant_file[board->en_passant_index % 8];
    }

    if (move_get_flag(move) == EN_PASSANT_FLAG) {
        int to_index = move_get_to_index(move);
        int from_index = move_get_from_index(move);
        int captured_index = board_get_piece_color(to_index, board) ? to_index - 8 : to_index + 8;
        board_set_piece(from_index, node.move_piece, board);
        board_set_piece(to_index, -1, board);
        board_set_piece(captured_index, node.captured_piece, board);
        return move;
    }
    
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
    PieceType old_type = board_get_piece(index, board);
    // Remove old piece
    if (board_get_piece_color(index, board)) {
        board->bit_boards[WHITE_PIECES] &= ~(1ULL << index);
    }
    else {
        board->bit_boards[BLACK_PIECES] &= ~(1ULL << index);
    }
    if (old_type != -1) {
        board->bit_boards[old_type] &= ~(1ULL << index);
        uint64_t zobrist_number = zobrist_table[index][piece_to_zobrist_index(old_type)];
        board->current_zobrist_hash ^= zobrist_number;
    }

    if (new_type == -1) {
        return;
    }

    // Set new piece
    board->bit_boards[new_type] |= (1ULL << index);
    uint64_t zobrist_number = zobrist_table[index][piece_to_zobrist_index(new_type)];
    board->current_zobrist_hash ^= zobrist_number;

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

Move* board_get_legal_captures(Board* board, AttackTable* attack_table, int* move_count) {
    return get_legal_captures(board, attack_table, move_count);
}

void board_change_turn(Board* board) {
    board->turn = !board->turn;
    board->current_zobrist_hash ^= zobrist_side_to_move;
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
    if (turn != board->turn) {
        board->current_zobrist_hash ^= zobrist_side_to_move;
        board->turn = turn;
    }
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
    free(board->undo_stack);
    free(board);
}

/* -------------------------- Internal functions --------------------------- */

void update_castling_rights(Move move, PieceType piece_type, Board* board) {
    int from_index = move_get_from_index(move);
    int to_index = move_get_to_index(move);

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

    switch(to_index) {
        case 0:
            board->castling_rights &= ~WHITE_CASTLE_QUEEN;
            break;
        case 7:
            board->castling_rights &= ~WHITE_CASTLE_KING;
            break;
        case 56:
            board->castling_rights &= ~BLACK_CASTLE_QUEEN;
            break;
        case 63:
            board->castling_rights &= ~BLACK_CASTLE_KING;
            break;
        default:
            break;
    }
}

void make_en_passant_move(Move move, PieceType piece_type,  Board* board) {
    int from_index = move_get_from_index(move);
    int to_index = move_get_to_index(move);
    int captured_index = board_get_piece_color(from_index, board) ? to_index - 8 : to_index + 8;
    board_set_piece(from_index, -1, board);
    board_set_piece(captured_index, -1, board);
    board_set_piece(to_index, piece_type, board);
}

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
    int to_index = move_get_to_index(move);
    int from_index = move_get_from_index(move);
    new_node.move_piece = (int8_t) board_get_piece(from_index, board);
    new_node.move = move;
    new_node.castling_rights = board->castling_rights;
    new_node.en_passant_index = board->en_passant_index;

    int captured_index;
    if (move_get_flag(move) == EN_PASSANT_FLAG) {
        captured_index = board_get_piece_color(from_index, board) ? to_index - 8 : to_index + 8;
    }
    else {
        captured_index = to_index;
    }
    new_node.captured_piece = board_get_piece(captured_index, board);

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

