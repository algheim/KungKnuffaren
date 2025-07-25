/*
 * @brief Generating legal movs.
 * 
 * 
 * 
 * @file movegenerator.c
 * @author Algot Heimrson
 * @date 2025-02-01
 */

#include "board.h"
#include "movegenerator.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "bitboard.h"

#define MAX_LEGAL_MOVES 218

uint64_t get_knight_moves(uint64_t friendly_pieces, uint64_t attacks);
uint64_t get_rook_moves(uint64_t friendly_pieces, uint64_t enemy_pieces, uint64_t attacks, int from_index);
uint64_t get_bishop_moves(uint64_t friendly_pieces, uint64_t enemy_pieces, uint64_t attacks, int from_index);
uint64_t get_queen_moves(uint64_t friendly_pieces, uint64_t enemy_pieces, uint64_t attacks, int from_index);
uint64_t get_king_moves(uint64_t friendly_pieces, uint64_t attacks);
uint64_t get_white_pawn_moves(Board* board, AttackTable* attack_table, int from_index);
uint64_t get_black_pawn_moves(Board* board, AttackTable* attack_table, int from_index);
uint64_t get_white_pawn_attacks(Board* board, AttackTable* attack_table, int from_index);
uint64_t get_black_pawn_attacks(Board* board, AttackTable* attack_table, int from_index);

uint64_t get_pinned_pieces(Board* board, int king_index, AttackTable* attack_table);
uint64_t get_pinned_msb(uint64_t pieces, Board* board, bool diagonal);
uint64_t get_pinned_lsb(uint64_t pieces, Board* board, bool diagonal);
int get_flag(PieceType piece, int from_index, int to_index);

void add_castle_moves(Board* board, Move* legal_moves, int* move_count, uint64_t attacked_squares);
uint64_t get_pseudo_attacks_from_index(Board* board, int index, AttackTable* attack_table);
uint64_t get_king_attackers(Board* board, int king_index, AttackTable* attack_table, uint64_t* all_attacks);
void get_moves_from_index(int from_index, uint64_t attacks, Move* moves, int* current_index, Board* board);
void get_moves_from_bit_board(Board* board, 
                              Move* moves, 
                              int* current_index, 
                              AttackTable* attack_table, 
                              uint64_t pinned_pieces,
                              uint64_t squares_blocking_king,
                              int king_index);

void add_en_passant_moves(Board* board, 
                          AttackTable* attack_table, 
                          Move* moves, int* move_count, 
                          uint64_t pinned_pieces, 
                          int king_index,
                          uint64_t squares_blocking_king);
bool check_en_passant_legality(Board* board, AttackTable* attack_table, int king_index, int from_index, int to_index);
static int max(int n1, int n2);
static int min(int n1, int n2);

#define WHITE_KINGSIDE_CASTLE_SAFE ((1ULL << 5) | (1ULL << 6))
#define WHITE_QUEENSIDE_CASTLE_SAFE ((1ULL << 2) | (1ULL << 3))
#define BLACK_KINGSIDE_CASTLE_SAFE ((1ULL << 61) | (1ULL << 62))
#define BLACK_QUEENSIDE_CASTLE_SAFE ((1ULL << 58) | (1ULL << 59))


/* -------------------------- External functions ----------------------------*/

/* This should be optimiezed later! */
Move* get_legal_captures(Board* board, AttackTable* attack_table, int* move_count) {
    int legal_move_count = 0;
    Move* legal_moves = get_legal_moves(board, attack_table, &legal_move_count);
    Move* legal_captures = calloc(MAX_LEGAL_MOVES + 1, sizeof(Move));
    uint64_t enemy_pieces = board->turn ? board->bit_boards[BLACK_PIECES] : board->bit_boards[WHITE_PIECES];

    for (int i = 0 ; i < legal_move_count ; i++) {
        if ((1ULL << move_get_to_index(legal_moves[i])) & enemy_pieces || move_get_flag(legal_moves[i]) == EN_PASSANT_FLAG) {
            legal_captures[(*move_count)++] = legal_moves[i];
        }
    }
    free(legal_moves);

    return legal_captures;
}

Move* get_legal_moves(Board* board, AttackTable* attack_table, int* move_count) {
    //printf("Generating legal moves for %s\n", board->turn ? "white" : "black");
    Move* legal_moves = calloc(MAX_LEGAL_MOVES + 1, sizeof(Move));
    uint64_t attacked_squares = 0ULL;
    int king_index;
    if (board->turn) {
        king_index = __builtin_ctzll(board->bit_boards[WHITE_KING]);
    }
    else {
        king_index = __builtin_ctzll(board->bit_boards[BLACK_KING]);
    }

    // We assume the king doesn't have to be blocked, so all squares 'blocks' the king.
    uint64_t squares_blocking_king = ~0ULL;
    uint64_t friendly_pieces = board->turn ? board->bit_boards[WHITE_PIECES] : board->bit_boards[BLACK_PIECES];

    // The king shouldn't block enemy sliding pieces, so I remove it :)
    board_set_piece(king_index, -1, board);
    uint64_t king_attackers = get_king_attackers(board, king_index, attack_table, &attacked_squares);
    board_set_piece(king_index, board->turn ? WHITE_KING : BLACK_KING, board);

    uint64_t legal_king_moves = attack_table->king_table[king_index];
    legal_king_moves &= ~friendly_pieces;
    legal_king_moves &= ~attacked_squares;

    // King is checked
    if (king_attackers) {
        int attacker_index = __builtin_ctzll(king_attackers);
        king_attackers &= king_attackers - 1;

        // King is double checked. Only king moves can be legal.
        if (king_attackers) {
            get_moves_from_index(king_index, legal_king_moves, legal_moves, move_count, board);
            return legal_moves;
        }
        squares_blocking_king = bit_board_from_to(king_index, attacker_index);
        squares_blocking_king |= (1ULL << attacker_index);
        //bit_board_print(squares_blocking_king);
    }
    else {
        add_castle_moves(board, legal_moves, move_count, attacked_squares);
    }
    uint64_t pinned_pieces = get_pinned_pieces(board, king_index, attack_table);

    add_en_passant_moves(board, attack_table, legal_moves, move_count, pinned_pieces, king_index, squares_blocking_king);

    // Regular moves
    get_moves_from_bit_board(board, legal_moves, move_count, attack_table, pinned_pieces, squares_blocking_king, king_index);
    get_moves_from_index(king_index, legal_king_moves, legal_moves, move_count, board);

    return legal_moves;
}

/* -------------------------- Internal functions ----------------------------*/

void add_en_passant_moves(Board* board, 
                          AttackTable* attack_table, 
                          Move* moves, int* move_count, 
                          uint64_t pinned_pieces, 
                          int king_index,
                          uint64_t squares_blocking_king) {
    if (board->en_passant_index == -1) {
        return;
    }
    int en_passant_rank = board->en_passant_index / 8;
    if ((board->turn && en_passant_rank != 5) || (!board->turn && en_passant_rank != 2)) {
        return;
    }

    uint64_t pawns = board->turn ? board->bit_boards[WHITE_PAWN] : board->bit_boards[BLACK_PAWN];
    uint64_t* pawn_attack_table = board->turn ? attack_table->white_pawn_attack_table : attack_table->black_pawn_attack_table;
    int captured_index = board->turn ? board->en_passant_index - 8 : board->en_passant_index + 8;

    if (squares_blocking_king & (1ULL << captured_index)) {
        squares_blocking_king |= 1ULL << (board->en_passant_index);
    }

    while (pawns) {
        int from_index = __builtin_ctzll(pawns);
        pawns &= pawns - 1;

        uint64_t current_attacks = pawn_attack_table[from_index];
        uint64_t legal_move = current_attacks & (1ULL << (board->en_passant_index));
        legal_move &= squares_blocking_king;

        if (pinned_pieces & (1ULL << from_index)) {
            uint64_t pinned_ray = bit_board_get_line(king_index, from_index);
            legal_move &= pinned_ray;
        }

        if (legal_move) {
            if (check_en_passant_legality(board, attack_table, king_index, from_index, board->en_passant_index)) {
                Move en_passant_move = move_create(from_index, board->en_passant_index, EN_PASSANT_FLAG);
                moves[(*move_count)++] = en_passant_move;
            }
        }
    }
}

/* Checks the edge-case where the two pawns in en passant are "double-pinned" to the king.*/
bool check_en_passant_legality(Board* board, AttackTable* attack_table, int king_index, int from_index, int to_index) {
    if (from_index / 8 != king_index / 8) {
        return true;
    }

    int captured_index = board->turn ? to_index - 8 : to_index + 8;
    
    uint64_t right_ray = attack_table->ray_dir_table[max(from_index, captured_index)][RIGHT];
    uint64_t left_ray = attack_table->ray_dir_table[min(from_index, captured_index)][LEFT];
    uint64_t all_pieces = board->bit_boards[WHITE_PIECES] | board->bit_boards[BLACK_PIECES];

    right_ray &= all_pieces;
    left_ray &= all_pieces;

    if ((!right_ray) || (!left_ray)) {
        return true;
    }

    PieceType right_type = board_get_piece(__builtin_ctzll(right_ray), board);
    PieceType left_type = board_get_piece(63 - __builtin_clzll((left_ray)), board);

    bool right_is_enemy_slider = (right_type == (board->turn ? BLACK_ROOK : WHITE_ROOK)) ||
                                (right_type == (board->turn ? BLACK_QUEEN : WHITE_QUEEN));
    bool left_is_enemy_slider = (left_type == (board->turn ? BLACK_ROOK : WHITE_ROOK)) ||
                                (left_type == (board->turn ? BLACK_QUEEN : WHITE_QUEEN));

    bool right_is_king = (right_type == (board->turn ? WHITE_KING : BLACK_KING));
    bool left_is_king  = (left_type == (board->turn ? WHITE_KING : BLACK_KING));

    if ((right_is_king && left_is_enemy_slider) ||
        (left_is_king && right_is_enemy_slider)) {
        return false;
    }

    return true;
}

void add_castle_moves(Board* board, Move* legal_moves, int* move_count, uint64_t attacked_squares) {
    uint64_t all_pieces = board->bit_boards[BLACK_PIECES] | board->bit_boards[WHITE_PIECES];
    if (board->turn) {
        if (board_white_can_castle_king(board) && !(attacked_squares & WHITE_KINGSIDE_CASTLE_SAFE)
            && !(WHITE_KINGSIDE_CASTLE_SAFE & all_pieces)) {
            legal_moves[(*move_count)++] = move_create(4, 6, CASTLE_FLAG);
        }

        if (board_white_can_castle_queen(board) && !(attacked_squares & WHITE_QUEENSIDE_CASTLE_SAFE)
            && !(WHITE_QUEENSIDE_CASTLE_SAFE & all_pieces)) {

            if (!((1ULL << 1) & all_pieces)) {
                legal_moves[(*move_count)++] = move_create(4, 2, CASTLE_FLAG);
            }
        }
    } 
    else {
        if (board_black_can_castle_king(board) && !(attacked_squares & BLACK_KINGSIDE_CASTLE_SAFE)
            && !(BLACK_KINGSIDE_CASTLE_SAFE & all_pieces)) {
            legal_moves[(*move_count)++] = move_create(60, 62, CASTLE_FLAG);
        }

        if (board_black_can_castle_queen(board) && !(attacked_squares & BLACK_QUEENSIDE_CASTLE_SAFE)
            && !(BLACK_QUEENSIDE_CASTLE_SAFE & all_pieces)) {

            if (!((1ULL << 57) & all_pieces)) {
                legal_moves[(*move_count)++] = move_create(60, 58, CASTLE_FLAG);
            }
        }
    }
}

/* Gets real legal moves from the bitboard of current pieces. */
void get_moves_from_bit_board(Board* board, 
                              Move* moves, 
                              int* current_index, 
                              AttackTable* attack_table,
                              uint64_t pinned_pieces,
                              uint64_t squares_blocking_king,
                              int king_index) {
    PieceType current_piece;
    int from_index;
    uint64_t friendly_pieces= board->turn ? board->bit_boards[WHITE_PIECES] : board->bit_boards[BLACK_PIECES];
    uint64_t enemy_pieces = board->turn ? board->bit_boards[BLACK_PIECES] : board->bit_boards[WHITE_PIECES];
    uint64_t current_pieces = friendly_pieces;
    uint64_t current_attacks;
    
    while(current_pieces) {
        from_index = __builtin_ctzll(current_pieces);
        current_pieces &= current_pieces - 1;

        current_piece = board_get_piece(from_index, board);
        if (current_piece == WHITE_KING || current_piece == BLACK_KING) {
            continue;
        }

        switch (current_piece) {
            case WHITE_QUEEN:
            case BLACK_QUEEN:
                current_attacks = get_queen_moves(friendly_pieces, enemy_pieces, attack_table->queen_table[from_index], from_index);
                break;

            case WHITE_ROOK:
            case BLACK_ROOK:
                current_attacks = get_rook_moves(friendly_pieces, enemy_pieces, attack_table->rook_table[from_index], from_index);
                break;

            case WHITE_BISHOP:
            case BLACK_BISHOP:
                current_attacks = get_bishop_moves(friendly_pieces, enemy_pieces, attack_table->bishop_table[from_index], from_index);
                break;

            case WHITE_KNIGHT:
            case BLACK_KNIGHT: 
                current_attacks = get_knight_moves(friendly_pieces, attack_table->knight_table[from_index]);
                break;

            case WHITE_PAWN:
                current_attacks = get_white_pawn_moves(board, attack_table, from_index);
                break;

            case BLACK_PAWN:
                current_attacks = get_black_pawn_moves(board, attack_table,from_index);
                break;

            default:
                current_attacks = 0ULL;
                break;
        }

        current_attacks &= ~friendly_pieces;
        
        if (pinned_pieces & (1ULL << from_index)) {
            uint64_t pinned_ray = bit_board_get_line(king_index, from_index);
            current_attacks &= pinned_ray;
        }
        current_attacks &= squares_blocking_king;
        get_moves_from_index(from_index, current_attacks, moves, current_index, board);
    }
}

/*
 * @brief   Adds the moves from from_index to every set bit in the attacks board to the move array starting
 *          at the given current_index. Adds a non-exsting move to the end of the sequence, and updates 
 *          current_index to point at the position after the last move.
 */
void get_moves_from_index(int from_index, uint64_t attacks, Move* moves, int* current_index, Board* board) {
    while (attacks) {
        int to_index = __builtin_ctzll(attacks);
        attacks &= attacks - 1;
        PieceType from_piece = board_get_piece(from_index, board);

        int flag = get_flag(from_piece, from_index, to_index);
        
        if (flag == QUEEN_PROMOTION_FLAG) {
            moves[(*current_index)++] = move_create(from_index, to_index, QUEEN_PROMOTION_FLAG);
            moves[(*current_index)++] = move_create(from_index, to_index, ROOK_PROMOTION_FLAG);
            moves[(*current_index)++] = move_create(from_index, to_index, BISHOP_PROMOTION_FLAG);
            moves[(*current_index)++] = move_create(from_index, to_index, KNIGHT_PROMOTION_FLAG);
        }
        else {
            moves[(*current_index)++] = move_create(from_index, to_index, flag);
        }
    }

    moves[*current_index] = move_create(0, 0, 0);
}

int get_flag(PieceType piece, int from_index, int to_index) {
    if (piece != WHITE_PAWN && piece != BLACK_PAWN) {
        return NORMAL_MOVE_FLAG;
    }

    if (abs(from_index - to_index) == 16) {
        return EN_PASSANT_AVAILABLE_FLAG;
    }

    if (to_index / 8 == 0 || to_index / 8 == 7) {
        return QUEEN_PROMOTION_FLAG;
    }

    return NORMAL_MOVE_FLAG;
}


uint64_t get_king_attackers(Board* board, int king_index, AttackTable* attack_table, uint64_t* all_attacks) {
    uint64_t king_attackers = 0ULL;
    uint64_t opponent_pieces;

    if (board->turn)
    {
        opponent_pieces = board->bit_boards[BLACK_PIECES];
    }
    else
    {
        opponent_pieces = board->bit_boards[WHITE_PIECES];
    }

    // Change turn to get opponent attacks instead of normal attacks.
    board_change_turn(board);
    while (opponent_pieces)
    {
        int current_index = __builtin_ctzll(opponent_pieces);
        opponent_pieces &= opponent_pieces - 1;

        uint64_t current_attack_board = get_pseudo_attacks_from_index(board, current_index, attack_table);
        (*all_attacks) = (*all_attacks) | (current_attack_board);

        if (current_attack_board & 1ULL << king_index) {
            king_attackers |= (1ULL << current_index);
        }
    }
    board_change_turn(board);

    return king_attackers;
}

uint64_t get_pseudo_attacks_from_index(Board* board, int index, AttackTable* attack_table) {
    PieceType piece_type = board_get_piece(index, board);
    uint64_t attacks = 0ULL;

    uint64_t friendly_pieces = board->turn ? board->bit_boards[WHITE_PIECES] : board->bit_boards[BLACK_PIECES];
    uint64_t enemy_pieces = board->turn ? board->bit_boards[BLACK_PIECES] : board->bit_boards[WHITE_PIECES];

    switch (piece_type) {
        case WHITE_QUEEN:
        case BLACK_QUEEN:
            attacks = get_queen_moves(friendly_pieces, enemy_pieces, attack_table->queen_table[index], index);
            break;

        case WHITE_ROOK:
        case BLACK_ROOK:
            attacks = get_rook_moves(friendly_pieces, enemy_pieces, attack_table->rook_table[index], index);
            break;

        case WHITE_BISHOP:
        case BLACK_BISHOP:
            attacks = get_bishop_moves(friendly_pieces, enemy_pieces, attack_table->bishop_table[index], index);
            break;

        case WHITE_KNIGHT:
        case BLACK_KNIGHT: 
            attacks = get_knight_moves(friendly_pieces, attack_table->knight_table[index]);
            break;

        case WHITE_PAWN:
            attacks = get_white_pawn_attacks(board, attack_table, index);
            break;

        case BLACK_PAWN:
            attacks = get_black_pawn_attacks(board, attack_table, index);
            break;

        default:
            break;
    }    

    return attacks;
}

uint64_t get_pinned_lsb(uint64_t pieces, Board* board, bool diagonal) {
    if (__builtin_popcountll(pieces) < 2)
        return 0ULL;

    int first_piece = __builtin_ctzll(pieces);
    pieces &= pieces - 1;
    int second_piece = __builtin_ctzll(pieces);

    bool first_is_friendly = board_get_piece_color(first_piece, board) == board->turn;
    bool second_is_enemy = board_get_piece_color(second_piece, board) != board->turn;

    if (first_is_friendly && second_is_enemy) {
        PieceType type = board_get_piece(second_piece, board);
        if (diagonal) {
            if (type == BLACK_QUEEN || type == WHITE_QUEEN || type == BLACK_BISHOP || type == WHITE_BISHOP) {
                return (1ULL << first_piece);
            }
        }
        else {
            if (type == BLACK_QUEEN || type == WHITE_QUEEN || type == BLACK_ROOK || type == WHITE_ROOK) {
                return (1ULL << first_piece);
            }
        }
    }

    return 0ULL;
}

uint64_t get_pinned_msb(uint64_t pieces, Board* board, bool diagonal) {
    if (__builtin_popcountll(pieces) < 2)
        return 0ULL;

    int first_piece = 63 - __builtin_clzll(pieces);
    pieces ^= (1ULL << first_piece);
    int second_piece = 63 - __builtin_clzll(pieces);

    bool first_is_friendly = board_get_piece_color(first_piece, board) == board->turn;
    bool second_is_enemy = board_get_piece_color(second_piece, board) != board->turn;

    if (first_is_friendly && second_is_enemy) {
        PieceType type = board_get_piece(second_piece, board);
        if (diagonal) {
            if (type == BLACK_QUEEN || type == WHITE_QUEEN || type == BLACK_BISHOP || type == WHITE_BISHOP) {
                return (1ULL << first_piece);
            }
        }
        else {
            if (type == BLACK_QUEEN || type == WHITE_QUEEN || type == BLACK_ROOK || type == WHITE_ROOK) {
                return (1ULL << first_piece);
            }
        }
    }

    return 0ULL;
}

uint64_t get_pinned_pieces(Board* board, int king_index, AttackTable* attack_table) {
    uint64_t pinned_pieces = 0ULL;
    uint64_t all_pieces = board->bit_boards[WHITE_PIECES] | board->bit_boards[BLACK_PIECES];
    uint64_t pieces;

    // Up
    pieces = attack_table->ray_dir_table[king_index][UP] & all_pieces;
    pinned_pieces |= get_pinned_lsb(pieces, board, false);

    // Up right
    pieces = attack_table->ray_dir_table[king_index][UP_RIGHT] & all_pieces;
    pinned_pieces |= get_pinned_lsb(pieces, board, true);

    // Right    
    pieces = attack_table->ray_dir_table[king_index][RIGHT] & all_pieces;
    pinned_pieces |= get_pinned_lsb(pieces, board, false);

    // Down right
    pieces = attack_table->ray_dir_table[king_index][DOWN_RIGHT] & all_pieces;
    pinned_pieces |= get_pinned_msb(pieces, board, true);

    // Down
    pieces = attack_table->ray_dir_table[king_index][DOWN] & all_pieces;
    pinned_pieces |= get_pinned_msb(pieces, board, false);

    // Down left
    pieces = attack_table->ray_dir_table[king_index][DOWN_LEFT] & all_pieces;
    pinned_pieces |= get_pinned_msb(pieces, board, true);

    // Left
    pieces = attack_table->ray_dir_table[king_index][LEFT] & all_pieces;
    pinned_pieces |= get_pinned_msb(pieces, board, false);

    // Up left
    pieces = attack_table->ray_dir_table[king_index][UP_LEFT] & all_pieces;
    pinned_pieces |= get_pinned_lsb(pieces, board, true);

    return pinned_pieces;
}

uint64_t get_knight_moves(uint64_t friendly_pieces, uint64_t attacks) {
    return attacks;// & (~friendly_pieces);
}

uint64_t get_rook_moves(uint64_t friendly_pieces, uint64_t enemy_pieces, uint64_t attacks, int from_index) {
    uint64_t legal_moves = 0;
    uint64_t all_pieces = friendly_pieces | enemy_pieces;

    // Up:
    int current_index = from_index + 8;
    while (current_index < 64) {
        legal_moves |= 1ULL << current_index;
        if (all_pieces & (1ULL << current_index)) {
            break;
        }
        current_index += 8;
    }

    // Down:
    current_index = from_index - 8;
    while (current_index >= 0) {
        legal_moves |= 1ULL << current_index;
        if (all_pieces & (1ULL << current_index)) {
            break;
        }
        current_index -= 8;
    }

    // Left:
    current_index = from_index - 1;
    while (current_index >= 0 && (current_index % 8) != 7) {
        legal_moves |= 1ULL << current_index;
        if (all_pieces & (1ULL << current_index)) {
            break;
        }
        current_index--;
    }

    // Right:
    current_index = from_index + 1;
    while (current_index < 64 && (current_index % 8) != 0) {
        legal_moves |= 1ULL << current_index;
        if (all_pieces & (1ULL << current_index)) {
            break;
        }
        current_index++;
    }

    //legal_moves &= (~friendly_pieces);
    //printf("ROOOK MOVES from %d: \n", from_index);
    //attack_table_print(legal_moves);
    return legal_moves;
}

uint64_t get_bishop_moves(uint64_t friendly_pieces, uint64_t enemy_pieces, uint64_t attacks, int from_index) {
    uint64_t legal_moves = 0;
    uint64_t all_pieces = friendly_pieces | enemy_pieces;

    // Down left:
    int current_index = from_index - 9;
    while (current_index >= 0 && (current_index % 8) != 7) {
        legal_moves |= 1ULL << current_index;
        if (all_pieces & (1ULL << current_index)) {
            break;
        }
        current_index -= 9;      
    }

    // Down right:
    current_index = from_index - 7;
    while (current_index >= 0 && (current_index % 8) != 0) {
        legal_moves |= 1ULL << current_index;
        if (all_pieces & (1ULL << current_index)) {
            break;
        }
        current_index -= 7;      
    }

    // Up right:
    current_index = from_index + 9;
    while (current_index < 64 && (current_index % 8) != 0) {
        legal_moves |= 1ULL << current_index;
        if (all_pieces & (1ULL << current_index)) {
            break;
        }
        current_index += 9;      
    }

    // Up left:
    current_index = from_index + 7;
    while (current_index < 64 && (current_index % 8) != 7) {
        legal_moves |= 1ULL << current_index;
        if (all_pieces & (1ULL << current_index)) {
            break;
        }
        current_index += 7;      
    }

    //legal_moves &= (~friendly_pieces);
    //printf("BISHOP MOVES from %d: \n", from_index);
    //attack_table_print(legal_moves);
    return legal_moves;
}

uint64_t get_queen_moves(uint64_t friendly_pieces, uint64_t enemy_pieces, uint64_t attacks, int from_index) {
    uint64_t rook_moves = get_rook_moves(friendly_pieces, enemy_pieces, attacks, from_index);
    uint64_t bishop_moves = get_bishop_moves(friendly_pieces, enemy_pieces, attacks, from_index);

    //printf("BISHOP MOVES from %d: \n", from_index);
    //attack_table_print(legal_moves);

    return rook_moves | bishop_moves;
}

uint64_t get_king_moves(uint64_t friendly_pieces, uint64_t attacks) {
    return attacks;// & (~friendly_pieces);
}

uint64_t get_white_pawn_moves(Board* board, AttackTable* attack_table, int from_index) {
    uint64_t normal_attacks = attack_table->white_pawn_table[from_index];
    uint64_t attack_attacks = attack_table->white_pawn_attack_table[from_index];
    uint64_t legal_moves = 0ULL;

    legal_moves |= normal_attacks & (~(board->bit_boards[WHITE_PIECES] | board->bit_boards[BLACK_PIECES]));
    legal_moves |= attack_attacks & (board->bit_boards[BLACK_PIECES]);

    if (from_index / 8 == 1) {
        if ((board->bit_boards[WHITE_PIECES] | board->bit_boards[BLACK_PIECES]) & (1ULL << (from_index + 8))) {
            legal_moves &= ~(1ULL << (from_index + 16));
        }
    }

    return legal_moves;
}

uint64_t get_black_pawn_moves(Board* board, AttackTable* attack_table, int from_index) {
    uint64_t normal_attacks = attack_table->black_pawn_table[from_index];
    uint64_t attack_attacks = attack_table->black_pawn_attack_table[from_index];
    uint64_t legal_moves = 0ULL;

    legal_moves |= normal_attacks & (~(board->bit_boards[WHITE_PIECES] | board->bit_boards[BLACK_PIECES]));
    legal_moves |= attack_attacks & (board->bit_boards[WHITE_PIECES]);

    if (from_index / 8 == 6) {
        if ((board->bit_boards[WHITE_PIECES] | board->bit_boards[BLACK_PIECES]) & (1ULL << (from_index - 8))) {
            legal_moves &= ~(1ULL << (from_index - 16));
        }
    }

    return legal_moves;
}

uint64_t get_white_pawn_attacks(Board* board, AttackTable* attack_table, int from_index) {
    uint64_t attack_attacks = attack_table->white_pawn_attack_table[from_index];
    return attack_attacks;
    uint64_t legal_moves = 0ULL;
    legal_moves |= attack_attacks & (board->bit_boards[BLACK_PIECES]);

    return legal_moves;
}

uint64_t get_black_pawn_attacks(Board* board, AttackTable* attack_table, int from_index) {
    uint64_t attack_attacks = attack_table->black_pawn_attack_table[from_index];
    return attack_attacks;
    uint64_t legal_moves = 0ULL;
    legal_moves |= attack_attacks & (board->bit_boards[WHITE_PIECES]);

    return legal_moves;
}

static int max(int n1, int n2) {
    return n1 > n2 ? n1 : n2;
}

static int min(int n1, int n2) {
    return n1 < n2 ? n1 : n2;
}