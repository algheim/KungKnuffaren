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

uint64_t get_pinned_pieces(Board* board, int king_index, AttackTable* attack_table);
uint64_t get_pinned_msb(uint64_t pieces, Board* board, bool diagonal);
uint64_t get_pinned_lsb(uint64_t pieces, Board* board, bool diagonal);

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

/* -------------------------- External functions ----------------------------*/

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

    // Legal king moves
    uint64_t king_attackers = get_king_attackers(board, king_index, attack_table, &attacked_squares);
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
    }

    // Regular moves
    uint64_t pinned_pieces = get_pinned_pieces(board, king_index, attack_table);
    get_moves_from_bit_board(board, legal_moves, move_count, attack_table, pinned_pieces, squares_blocking_king, king_index);
    get_moves_from_index(king_index, legal_king_moves, legal_moves, move_count, board);

    return legal_moves;
}

/* -------------------------- Internal functions ----------------------------*/


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
        PieceType to_piece = board_get_piece(to_index, board);
        moves[*current_index] = move_create(from_index, to_index, from_piece, to_piece);

        (*current_index)++;
    }

    moves[*current_index] = move_create(-1, -1, -1, -1);
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
            attacks = get_white_pawn_moves(board, attack_table, index);
            break;

        case BLACK_PAWN:
            attacks = get_black_pawn_moves(board, attack_table, index);
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
    uint64_t legal_moves = 0;

    legal_moves |= normal_attacks & (~(board->bit_boards[WHITE_PIECES] | board->bit_boards[BLACK_PIECES]));
    legal_moves |= attack_attacks & (board->bit_boards[BLACK_PIECES]);

    if (from_index / 8 == 1) {
        if ((board->bit_boards[WHITE_PIECES] | board->bit_boards[BLACK_PIECES]) & (1ULL << (from_index + 8))) {
            legal_moves ^= 1ULL << (from_index + 16);
        }
    }

    return legal_moves;
}

uint64_t get_black_pawn_moves(Board* board, AttackTable* attack_table, int from_index) {
    uint64_t normal_attacks = attack_table->black_pawn_table[from_index];
    uint64_t attack_attacks = attack_table->black_pawn_attack_table[from_index];
    uint64_t legal_moves = 0;

    legal_moves |= normal_attacks & (~(board->bit_boards[WHITE_PIECES] | board->bit_boards[BLACK_PIECES]));
    legal_moves |= attack_attacks & (board->bit_boards[WHITE_PIECES]);

    if (from_index / 8 == 6) {
        if ((board->bit_boards[WHITE_PIECES] | board->bit_boards[BLACK_PIECES]) & (1ULL << (from_index - 8))) {
            legal_moves ^= 1ULL << (from_index - 16);
        }
    }

    return legal_moves;
}
