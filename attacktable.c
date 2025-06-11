#include "attacktable.h"
#include "stdlib.h"
#include <stdio.h>

uint64_t set_bit(uint64_t bit_board, int x, int y);
uint64_t get_knight_board(int x, int y);
uint64_t get_king_board(int x, int y);
uint64_t get_rook_board(int x, int y);
uint64_t get_bishop_board(int x, int y);
uint64_t get_queen_board(int x, int y);
uint64_t get_white_pawn_board(int x, int y);
uint64_t get_black_pawn_board(int x, int y);
uint64_t get_white_pawn_attack_board(int x, int y);
uint64_t get_black_pawn_attack_board(int x, int y);

uint64_t set_square_dir_table(AttackTable* attack_table);

/* --------------------- External functions ---------------------*/

AttackTable* attack_table_create() {
    AttackTable* attack_table = malloc(sizeof(AttackTable));

    for (int i = 0 ; i < 64 ; i++) {
        int x = i % 8;
        int y = i / 8;

        attack_table->knight_table[i] = get_knight_board(x, y);
        attack_table->king_table[i] = get_king_board(x, y);
        attack_table->rook_table[i] = get_rook_board(x, y);
        attack_table->bishop_table[i] = get_bishop_board(x, y);
        attack_table->queen_table[i] = get_queen_board(x, y);
        attack_table->white_pawn_table[i] = get_white_pawn_board(x, y);
        attack_table->black_pawn_table[i] = get_black_pawn_board(x, y);
        attack_table->white_pawn_attack_table[i] = get_white_pawn_attack_board(x, y);
        attack_table->black_pawn_attack_table[i] = get_black_pawn_attack_board(x, y);
    }

    set_square_dir_table(attack_table);

    return attack_table;
}

uint64_t attack_table_get_board(int index, PieceType piece_type, AttackTable* attack_table) {
    switch (piece_type) {
        case WHITE_KING:
        case BLACK_KING:
            return attack_table->king_table[index];

        case WHITE_QUEEN:
        case BLACK_QUEEN:
            return attack_table->queen_table[index];

        case WHITE_ROOK:
        case BLACK_ROOK:
            return attack_table->rook_table[index];

        case WHITE_BISHOP:
        case BLACK_BISHOP:
            return attack_table->bishop_table[index];

        case WHITE_KNIGHT:
        case BLACK_KNIGHT:
            return attack_table->knight_table[index];

        case WHITE_PAWN:
            return attack_table->white_pawn_attack_table[index];

        case BLACK_PAWN:
            return attack_table->black_pawn_attack_table[index];

        default:
            return 0ULL;
    }
}


/**
 * @brief Prints the given bit board as an 8x8 square with coordinate 0,0 
 * 
 * @param bit_board The bit board.
 */
void attack_table_print(uint64_t bit_board) {
    printf("--------\n");
    for (int row = 7 ; row >= 0 ; row--) {
        for (int file = 0 ; file < 8 ; file ++) {
            int square = row * 8 + file;
            printf("%c", (bit_board & (1ULL << square)) ? '1' : '0');
        }

        printf("\n");
    }
    printf("--------\n");

}

void attack_table_print2(uint64_t bit_board) {
    for (int i = 0 ; i <  64 ; i++) {
        if (i % 8 == 0) {
            printf("\n");
        }
        if (bit_board & (1ULL << (63 - i))) {
            printf("1");
        }
        else {
            printf("0");
        }
    }

    printf("\n");
}

void attack_table_destroy(AttackTable* attack_table) {
    free (attack_table);
}

/* ------------------- Internal functions --------------------*/

/**
 * @brief   If the given coordinate is in range, sets the bit at the coordinate
 *          to 1.
 * 
 * @param bit_board The bit-board.
 * @param x         The x-coordinate.
 * @param y         The y-coordinate. 
 */
uint64_t set_bit(uint64_t bit_board, int x, int y) {
    if ((x < 0) || (x > 7) || (y < 0) || (y > 7)) {
        return bit_board;
    }
    int shifts = y * 8 + x;

    return bit_board | 1ULL << shifts;
}

uint64_t get_knight_board(int x, int y) {
    uint64_t knight_board = 0;
    
    // Up
    knight_board = set_bit(knight_board, x - 1, y - 2);
    knight_board = set_bit(knight_board, x + 1, y - 2);

    // Right
    knight_board = set_bit(knight_board, x + 2, y - 1);
    knight_board = set_bit(knight_board, x + 2, y + 1);

    // Left
    knight_board = set_bit(knight_board, x - 2, y - 1);
    knight_board = set_bit(knight_board, x - 2, y + 1);

    // Down
    knight_board = set_bit(knight_board, x - 1, y + 2);
    knight_board = set_bit(knight_board, x + 1, y + 2);

    return knight_board;
}

uint64_t get_king_board(int x, int y) {
    uint64_t king_board = 0;

    king_board = set_bit(king_board, x - 1, y - 1);
    king_board = set_bit(king_board, x - 1, y);
    king_board = set_bit(king_board, x - 1, y + 1);

    king_board = set_bit(king_board, x, y - 1);
    king_board = set_bit(king_board, x, y + 1);

    king_board = set_bit(king_board, x + 1, y - 1);
    king_board = set_bit(king_board, x + 1, y);
    king_board = set_bit(king_board, x + 1, y + 1);

    return king_board;
}

uint64_t get_rook_board(int x, int y) {
    uint64_t rook_board = 0;

    // Up
    for (int i = y - 1 ; i >= 0 ; i--) {
        rook_board = set_bit(rook_board, x, i);
    }

    // Right
    for (int i = x + 1 ; i < 8 ; i++) {
        rook_board = set_bit(rook_board, i, y);
    }

    // Down
    for (int i = y + 1 ; i < 8 ; i++) {
        rook_board = set_bit(rook_board, x, i);
    }

    // Left
    for (int i = x - 1 ; i >= 0 ; i--) {
        rook_board = set_bit(rook_board, i, y);
    }

    return rook_board;
}

uint64_t get_bishop_board(int x, int y) {
    uint64_t bishop_board = 0;

    // Up Right
    int temp_x = x + 1;
    int temp_y = y - 1;
    while ((temp_x < 8) && (temp_y >= 0)) {
        bishop_board = set_bit(bishop_board, temp_x, temp_y);
        temp_x++;
        temp_y--;
    }

    // Up Left
    temp_x = x - 1;
    temp_y = y - 1;
    while ((temp_x >= 0) && (temp_y >= 0)) {
        bishop_board = set_bit(bishop_board, temp_x, temp_y);
        temp_x--;
        temp_y--;
    }

    // Down Right
    temp_x = x + 1;
    temp_y = y + 1;
    while ((temp_x < 8) && (temp_y < 8)) {
        bishop_board = set_bit(bishop_board, temp_x, temp_y);
        temp_x++;
        temp_y++;
    }

    // Down Left
    temp_x = x - 1;
    temp_y = y + 1;
    while ((temp_x >= 0) && (temp_y < 8)) {
        bishop_board = set_bit(bishop_board, temp_x, temp_y);
        temp_x--;
        temp_y++;
    }

    return bishop_board;
}

uint64_t get_queen_board(int x, int y) {
    uint64_t rook_board = get_rook_board(x, y);
    uint64_t bishop_board = get_bishop_board(x, y);

    return rook_board | bishop_board;
}

uint64_t get_white_pawn_board(int x, int y) {
    uint64_t white_pawn_board = 0;

    white_pawn_board = set_bit(white_pawn_board, x, y + 1);

    if (y == 1) {
        white_pawn_board = set_bit(white_pawn_board, x, y + 2);
    }

    return white_pawn_board;
}

uint64_t get_black_pawn_board(int x, int y) {
    uint64_t black_pawn_board = 0;

    black_pawn_board = set_bit(black_pawn_board, x, y - 1);

    if (y == 6) {
        black_pawn_board = set_bit(black_pawn_board, x, y - 2);
    }

    return black_pawn_board;
}

uint64_t get_white_pawn_attack_board(int x, int y) {
    uint64_t white_pawn_attack_board = 0;

    white_pawn_attack_board = set_bit(white_pawn_attack_board, x + 1, y + 1);
    white_pawn_attack_board = set_bit(white_pawn_attack_board, x - 1, y + 1);

    return white_pawn_attack_board;
}

uint64_t get_black_pawn_attack_board(int x, int y) {
    uint64_t black_pawn_attack_board = 0;

    black_pawn_attack_board = set_bit(black_pawn_attack_board, x + 1, y - 1);
    black_pawn_attack_board = set_bit(black_pawn_attack_board, x - 1, y - 1);

    return black_pawn_attack_board;
}


uint64_t set_square_dir_table(AttackTable* attack_table) {
    for (int i = 0 ; i < 64 ; i++) {
        int x = i % 8;
        int y = i / 8;
        
        uint64_t up_board = 0;
        uint64_t up_right_board= 0;
        uint64_t right_board = 0;
        uint64_t down_right_board = 0;
        uint64_t down_board = 0;
        uint64_t down_left_board = 0;
        uint64_t left_board = 0;
        uint64_t up_left_board = 0;
    
        // Up
        for (int i = y - 1 ; i >= 0 ; i--) {
            up_board |= set_bit(up_board, x, i);
        }

        // Right
        for (int i = x + 1 ; i < 8 ; i++) {
            right_board = set_bit(right_board, i, y);
        }

        // Down
        for (int i = y + 1 ; i < 8 ; i++) {
            down_board = set_bit(down_board, x, i);
        }

        // Left
        for (int i = x - 1 ; i >= 0 ; i--) {
            left_board = set_bit(left_board, i, y);
        }

        // Up Right
        int temp_x = x + 1;
        int temp_y = y - 1;
        while ((temp_x < 8) && (temp_y >= 0)) {
            up_right_board = set_bit(up_right_board, temp_x, temp_y);
            temp_x++;
            temp_y--;
        }

        // Up Left
        temp_x = x - 1;
        temp_y = y - 1;
        while ((temp_x >= 0) && (temp_y >= 0)) {
            up_left_board = set_bit(up_left_board, temp_x, temp_y);
            temp_x--;
            temp_y--;
        }

        // Down Right
        temp_x = x + 1;
        temp_y = y + 1;
        while ((temp_x < 8) && (temp_y < 8)) {
            down_right_board = set_bit(down_right_board, temp_x, temp_y);
            temp_x++;
            temp_y++;
        }

        // Down Left
        temp_x = x - 1;
        temp_y = y + 1;
        while ((temp_x >= 0) && (temp_y < 8)) {
            down_left_board = set_bit(down_left_board, temp_x, temp_y);
            temp_x--;
            temp_y++;
        }

        attack_table->ray_dir_table[i][UP] = up_board;
        attack_table->ray_dir_table[i][UP_RIGHT] = up_right_board;
        attack_table->ray_dir_table[i][RIGHT] = right_board;
        attack_table->ray_dir_table[i][DOWN_RIGHT] = down_right_board;
        attack_table->ray_dir_table[i][DOWN] = down_board;
        attack_table->ray_dir_table[i][DOWN_LEFT] = down_left_board;
        attack_table->ray_dir_table[i][LEFT] = left_board;
        attack_table->ray_dir_table[i][UP_LEFT] = up_left_board;
    }
}

