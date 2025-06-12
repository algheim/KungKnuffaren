


#ifndef BITBOARD_H
#define BITBOARD_H

#include "stdint.h"

int bit_board_get_lsb_index(uint64_t bit_board);

int bit_board_get_msb_index(uint64_t bit_board);

void bit_board_print(uint64_t bit_board);

uint64_t bit_board_set_bit(uint64_t bit_board, int index);

uint64_t bit_board_from_to(int from, int to);

uint64_t bit_board_get_line(int from, int to);

uint64_t bit_board_up_ray(int from, int count);

uint64_t bit_board_down_ray(int from, int count);

uint64_t bit_board_right_ray(int from, int count);

uint64_t bit_board_left_ray(int from, int count);

uint64_t bit_board_up_right_ray(int from, int count);

uint64_t bit_board_up_left_ray(int from, int count);

uint64_t bit_board_down_right_ray(int from, int count);

uint64_t bit_board_down_left_ray(int from, int count);


#endif
