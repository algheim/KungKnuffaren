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

#define MAX_LEGAL_MOVES 218

Move* get_pseudo_legal_white_moves(Board* board, AttackTable* attack_table);
Move* get_pseudo_legal_black_moves(Board* board, AttackTable* attack_table);

PieceType get_white_piece_type(Board* board, int index);
PieceType get_black_piece_type(Board* borad, int index);

uint64_t get_knight_moves(uint64_t friendly_pieces, uint64_t attacks);
uint64_t get_rook_moves(uint64_t friendly_pieces, uint64_t enemy_pieces, uint64_t attacks, int from_index);
uint64_t get_bishop_moves(uint64_t friendly_pieces, uint64_t enemy_pieces, uint64_t attacks, int from_index);
uint64_t get_queen_moves(uint64_t friendly_pieces, uint64_t enemy_pieces, uint64_t attacks, int from_index);
uint64_t get_king_moves(uint64_t friendly_pieces, uint64_t attacks);
uint64_t get_white_pawn_moves(Board* board, AttackTable* attack_table, int from_index);
uint64_t get_black_pawn_moves(Board* board, AttackTable* attack_table, int from_index);


Move* clean_move_array(Board* board, Move* move_array);
uint64_t get_pinned_pieces(Board* board, int king_index, AttackTable* attack_table);
uint64_t get_pinned_msb(uint64_t pieces, Board* board);
uint64_t get_pinned_lsb(uint64_t pieces, Board* board);

/* -------------------------- External functions ----------------------------*/

Move* get_legal_moves(Board* board, AttackTable* attack_table) {
    Move* pseudo_legal_moves;
    int king_index;
    uint64_t attacked_squares;
    
    if (board->turn) {
        pseudo_legal_moves = get_pseudo_legal_white_moves(board, attack_table);
        king_index = __builtin_ctzll(board->bit_boards[WHITE_KING]);
        attacked_squares = get_pseudo_legal_black_moves(board, attack_table);
    }
    else {
        pseudo_legal_moves = get_pseudo_legal_black_moves(board, attack_table);
        king_index = __builtin_ctzll(board->bit_boards[BLACK_KING]);
        attacked_squares = get_pseudo_legal_white_moves(board, attack_table);
    }

    uint64_t king_attackers = get_king_attackers(board, king_index, attack_table);
    uint64_t pinned_pieces = get_pinned_pieces(board, king_index, attack_table);
    
}

/* -------------------------- Internal functions ----------------------------*/

uint64_t get_pinned_lsb(uint64_t pieces, Board* board) {
    uint64_t pinned = 0;

    int first_piece = __builtin_ctzll(pieces);
    pieces &= pieces - 1;
    int second_piece = __builtin_ctcll(pieces);

    if (board->turn) {
        if (board_get_piece_color(first_piece, board) && !board_get_piece_color(second_piece, board)) {
            PieceType type = board_get_piece(second_piece, board);
            if (type >= BLACK_QUEEN && type <= BLACK_BISHOP) {
                pinned |= (1ULL << second_piece);
            } 
        }
    }
    else {
        if (!board_get_piece_color(first_piece, board) && board_get_piece_color(second_piece, board)) {
            PieceType type = board_get_piece(second_piece, board);
            if (type >=WHITE_QUEEN && type <= WHITE_BISHOP) {
                pinned |= (1ULL << second_piece);
            } 
        }
    }
    
    return pinned;
}

uint64_t get_pinned_msb(uint64_t pieces, Board* board) {
    uint64_t pinned = 0;

    int first_piece = 63 - __builtin_clzll(pieces);
    pieces ^= 1ULL << (63 - __builtin_clzll(pieces));
    int second_piece = 63 - __builtin_clzll(pieces);

    if (board->turn) {
        if (board_get_piece_color(first_piece, board) && !board_get_piece_color(second_piece, board)) {
            PieceType type = board_get_piece(second_piece, board);
            if (type >= BLACK_QUEEN && type <= BLACK_BISHOP) {
                pinned |= (1ULL << second_piece);
            } 
        }
    }
    else {
        if (!board_get_piece_color(first_piece, board) && board_get_piece_color(second_piece, board)) {
            PieceType type = board_get_piece(second_piece, board);
            if (type >=WHITE_QUEEN && type <= WHITE_BISHOP) {
                pinned |= (1ULL << second_piece);
            } 
        }
    }
    
    return pinned;
}

uint64_t get_pinned_pieces(Board* board, int king_index, AttackTable* attack_table) {
    uint64_t pinned_pieces = 0;
    uint64_t all_pieces = board->bit_boards[WHITE_PIECES] | board->bit_boards[BLACK_PIECES];
    uint64_t pieces;

    // Up
    pieces = attack_table->ray_dir_table[king_index][UP] & all_pieces;
    pinned_pieces |= get_pinned_lsb(pieces, board->turn);

    // Up right
    pieces = attack_table->ray_dir_table[king_index][UP_RIGHT] & all_pieces;
    pinned_pieces |= get_pinned_lsb(pieces, board->turn);

    // Right    
    pieces = attack_table->ray_dir_table[king_index][RIGHT] & all_pieces;
    pinned_pieces |= get_pinned_lsb(pieces, board->turn);

    // Down right
    pieces = attack_table->ray_dir_table[king_index][DOWN_RIGHT] & all_pieces;
    pinned_pieces |= get_pinned_msb(pieces, board->turn);

    // Down
    pieces = attack_table->ray_dir_table[king_index][DOWN] & all_pieces;
    pinned_pieces |= get_pinned_msb(pieces, board->turn);

    // Down left
    pieces = attack_table->ray_dir_table[king_index][DOWN_LEFT] & all_pieces;
    pinned_pieces |= get_pinned_msb(pieces, board->turn);

    // Left
    pieces = attack_table->ray_dir_table[king_index][LEFT] & all_pieces;
    pinned_pieces |= get_pinned_msb(pieces, board->turn);

    // Up left
    pieces = attack_table->ray_dir_table[king_index][UP_LEFT] & all_pieces;
    pinned_pieces |= get_pinned_lsb(pieces, board->turn);

    return pinned_pieces;
}

/*
Planning to change so get_white_bishop_moves returns a Move array, to be able to store more info about each move.
*/
Move* get_pseudo_legal_white_moves(Board* board, AttackTable* attack_table) {
    Move* legal_moves = calloc(MAX_LEGAL_MOVES + 1, sizeof(Move));
    int current_move_index = 0;
    PieceType current_piece;
    int64_t current_pieces = board->bit_boards[WHITE_PIECES];

    while(current_pieces) {
        int from_index = __builtin_ctzll(current_pieces);
        uint64_t current_attacks = 0;

        // Determine what piece is at the index:
        if (board->bit_boards[WHITE_PAWN] & (1ULL << from_index)) {
            current_attacks = attack_table->white_pawn_table[from_index];
        }
        else if (board->bit_boards[WHITE_BISHOP] & (1ULL << from_index)) {
            current_piece = WHITE_BISHOP;
            current_attacks = get_bishop_moves(board->bit_boards[WHITE_PIECES],
                                               board->bit_boards[BLACK_PIECES],
                                               attack_table->rook_table[from_index], from_index);
        }
        else if (board->bit_boards[WHITE_ROOK] & (1ULL << from_index)) {
            current_piece = WHITE_ROOK;
            current_attacks = get_rook_moves(board->bit_boards[WHITE_PIECES], 
                                             board->bit_boards[BLACK_PIECES], 
                                             attack_table->rook_table[from_index], from_index);
        } 
        else if (board->bit_boards[WHITE_KING] & (1ULL << from_index)) {
            current_piece = WHITE_KING;
            current_attacks = get_king_moves(board->bit_boards[WHITE_PIECES], 
                                             attack_table->king_table[from_index]);
        }
        else if (board->bit_boards[WHITE_QUEEN] & (1ULL << from_index)) {
            current_piece = WHITE_QUEEN;
            current_attacks = get_queen_moves(board->bit_boards[WHITE_PIECES], 
                                              board->bit_boards[BLACK_PIECES], 
                                              attack_table->rook_table[from_index], from_index);
        }
        else if (board->bit_boards[WHITE_KNIGHT] & (1ULL << from_index)) {
            current_piece = WHITE_KNIGHT;
            current_attacks = get_knight_moves(board->bit_boards[WHITE_PIECES], 
                                               attack_table->knight_table[from_index]);
        }
        else if (board->bit_boards[WHITE_PAWN] & (1ULL << from_index)) {
            current_piece = WHITE_PAWN;
            current_attacks = get_white_pawn_moves(board, attack_table, from_index);
        }

        current_pieces &= current_pieces - 1;

        // Add attacks to move array.
        while (current_attacks) {
            int to_index = __builtin_ctzll(current_attacks);
            //printf("From:  %d\t To: %d\n", from_index, to_index);
            current_attacks &= current_attacks - 1;
            PieceType to_piece = get_black_piece_type(board, to_index);
            legal_moves[current_move_index] = move_create(from_index, to_index, current_piece, to_piece);


            current_move_index++;
        }
        legal_moves[current_move_index] = move_create(-1, -1, -1, -1);
    }
    
    return legal_moves;
}

Move* get_pseudo_legal_black_moves(Board* board, AttackTable* attack_table) {
    Move* legal_moves = calloc(MAX_LEGAL_MOVES + 1, sizeof(Move));
    int current_move_index = 0;
    PieceType current_piece;
    int64_t current_pieces = board->bit_boards[BLACK_PIECES];

    while(current_pieces) {
        int from_index = __builtin_ctzll(current_pieces);
        uint64_t current_attacks = 0;

        // Determine what piece is at the index:
        if (board->bit_boards[BLACK_PAWN] & (1ULL << from_index)) {
            current_attacks = attack_table->black_pawn_table[from_index];
        }
        else if (board->bit_boards[BLACK_BISHOP] & (1ULL << from_index)) {
            current_piece = BLACK_BISHOP;
            current_attacks = get_rook_moves(board->bit_boards[BLACK_PIECES], 
                                             board->bit_boards[WHITE_PIECES], 
                                             attack_table->rook_table[from_index], from_index);
        }
        else if (board->bit_boards[BLACK_ROOK] & (1ULL << from_index)) {
            current_piece = BLACK_ROOK;
            current_attacks = get_rook_moves(board->bit_boards[BLACK_PIECES], 
                                             board->bit_boards[WHITE_PIECES], 
                                             attack_table->rook_table[from_index], from_index);
        } 
        else if (board->bit_boards[BLACK_KING] & (1ULL << from_index)) {
            current_piece = BLACK_KING;
            current_attacks = attack_table->king_table[from_index];
        }
        else if (board->bit_boards[BLACK_QUEEN] & (1ULL << from_index)) {
            current_piece = BLACK_QUEEN;
            current_attacks = get_queen_moves(board->bit_boards[BLACK_PIECES], 
                                              board->bit_boards[WHITE_PIECES], 
                                              attack_table->rook_table[from_index], from_index);
        }
        else if (board->bit_boards[BLACK_KNIGHT] & (1ULL << from_index)) {
            current_piece = BLACK_KNIGHT;
            current_attacks = get_knight_moves(board->bit_boards[BLACK_PIECES], 
                                               attack_table->knight_table[from_index]);
        }
        else if (board->bit_boards[BLACK_PAWN] & (1ULL << from_index)) {
            current_piece = BLACK_PAWN;
            current_attacks = get_white_pawn_moves(board, attack_table, from_index);
        }

        current_pieces &= current_pieces - 1;

        // Add attacks to move array.
        while (current_attacks) {
            int to_index = __builtin_ctzll(current_attacks);
            //printf("From:  %d\t To: %d\n", from_index, to_index);
            current_attacks &= current_attacks - 1;
            PieceType to_piece = get_white_piece_type(board, to_index);
            legal_moves[current_move_index] = move_create(from_index, to_index, current_piece, to_piece);

            current_move_index++;
        }
        legal_moves[current_move_index] = move_create(-1, -1, -1, -1);
    }
    
    return legal_moves;
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

uint64_t get_knight_moves(uint64_t friendly_pieces, uint64_t attacks) {
    return attacks & (~friendly_pieces);

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

    legal_moves &= (~friendly_pieces);
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

    legal_moves &= (~friendly_pieces);
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
    return attacks & (~friendly_pieces);
}

uint64_t get_white_pawn_moves(Board* board, AttackTable* attack_table, int from_index) {
    uint64_t normal_attacks = attack_table->white_pawn_table[from_index];
    uint64_t attack_attacks = attack_table->white_pawn_attack_table[from_index];
    uint64_t legal_moves = 0;

    legal_moves |= normal_attacks & (~(board->bit_boards[WHITE_PIECES] | board->bit_boards[BLACK_PIECES]));
    legal_moves |= attack_attacks & (board->bit_boards[BLACK_PIECES]);

    return legal_moves;
}

uint64_t get_black_pawn_moves(Board* board, AttackTable* attack_table, int from_index) {
    uint64_t normal_attacks = attack_table->black_pawn_table[from_index];
    uint64_t attack_attacks = attack_table->black_pawn_attack_table[from_index];
    uint64_t legal_moves = 0;

    legal_moves |= normal_attacks & (~(board->bit_boards[WHITE_PIECES] | board->bit_boards[BLACK_PIECES]));
    legal_moves |= attack_attacks & (board->bit_boards[WHITE_PIECES]);

    return legal_moves;
}
