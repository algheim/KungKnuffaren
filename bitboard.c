
#include "bitboard.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

int min(int n1, int n2);

uint64_t bit_board_from_to(int from, int to) {
    int from_x = from % 8;
    int from_y = from / 8;
    int to_x = to % 8;
    int to_y = to / 8;

    if (to_x == from_x) {
        if (to_y > from_y) {
            return bit_board_up_ray(from, to_y - from_y); 
        }
        else if (to_y < from_y) {
            return bit_board_down_ray(from, from_y - to_y);
        }
    }

    if (to_y == from_y) {
        if (to_x > from_x) {
            return bit_board_right_ray(from, to_x - from_x);
        }
        else if (to_x < from_x) {
            return bit_board_left_ray(from, from_x - to_x);
        }
    }

    // Diagonals
    if (abs(to_x - from_x) == abs(to_y - from_y)) {
        int distance = abs(to_x - from_x);

        if (to_x > from_x && to_y > from_y) {
            return bit_board_up_right_ray(from, distance);
        } 
        else if (to_x < from_x && to_y > from_y) {
            return bit_board_up_left_ray(from, distance);
        } 
        else if (to_x > from_x && to_y < from_y) {
            return bit_board_down_right_ray(from, distance);
        } 
        else if (to_x < from_x && to_y < from_y) {
            return bit_board_down_left_ray(from, distance);
        }
    }

    return 0ULL;
}

uint64_t bit_board_get_line(int from, int to) {
    int from_x = from % 8;
    int from_y = from / 8;
    int to_x = to % 8;
    int to_y = to / 8;

    if (to_x == from_x) {
        if (to_y > from_y) {
            int distance = 7 - from_y;
            return bit_board_up_ray(from, distance); 
        } else if (to_y < from_y) {
            int distance = from_y;
            return bit_board_down_ray(from, distance);
        }
    }

    if (to_y == from_y) {
        if (to_x > from_x) {
            int distance = 7 - from_x;
            return bit_board_right_ray(from, distance);
        } else if (to_x < from_x) {
            int distance = from_x;
            return bit_board_left_ray(from, distance);
        }
    }

    // Diagonals
    if (abs(to_x - from_x) == abs(to_y - from_y)) {
        int distance;
        if (to_x > from_x && to_y > from_y) {
            distance = min(7 - from_x, 7 - from_y);
            return bit_board_up_right_ray(from, distance);
        } 
        else if (to_x < from_x && to_y > from_y) {
            distance = min(from_x, 7 - from_y);
            return bit_board_up_left_ray(from, distance);
        } 
        else if (to_x > from_x && to_y < from_y) {
            distance = min(7 - from_x, from_y);
            return bit_board_down_right_ray(from, distance);
        } 
        else if (to_x < from_x && to_y < from_y) {
            distance = min(from_x, from_y);
            return bit_board_down_left_ray(from, distance);
        }
    }

    return 0ULL;
}

uint64_t bit_board_up_ray(int from, int count) {
    uint64_t board = 0ULL;
    int current_index = from + 8;
    for (int i = 0; i < count && current_index < 64; i++) {
        board |= 1ULL << current_index;
        current_index += 8;
    }
    return board;
}

uint64_t bit_board_down_ray(int from, int count) {
    uint64_t board = 0ULL;
    int current_index = from - 8;
    for (int i = 0; i < count && current_index >= 0; i++) {
        board |= 1ULL << current_index;
        current_index -= 8;
    }
    return board;
}

uint64_t bit_board_right_ray(int from, int count) {
    uint64_t board = 0ULL;
    int current_index = from + 1;
    for (int i = 0; i < count && current_index % 8 != 0; i++) {
        board |= 1ULL << current_index;
        current_index += 1;
    }
    return board;
}

uint64_t bit_board_left_ray(int from, int count) {
    uint64_t board = 0ULL;
    int current_index = from - 1;
    for (int i = 0; i < count && from % 8 != 0 && current_index % 8 != 7; i++) {
        board |= 1ULL << current_index;
        current_index -= 1;
    }
    return board;
}

uint64_t bit_board_up_right_ray(int from, int count) {
    uint64_t board = 0ULL;
    int current_index = from + 9;
    for (int i = 0; i < count && current_index < 64 && current_index % 8 != 0; i++) {
        board |= 1ULL << current_index;
        current_index += 9;
    }
    return board;
}

uint64_t bit_board_up_left_ray(int from, int count) {
    uint64_t board = 0ULL;
    int current_index = from + 7;
    for (int i = 0; i < count && current_index < 64 && current_index % 8 != 7; i++) {
        board |= 1ULL << current_index;
        current_index += 7;
    }
    return board;
}

uint64_t bit_board_down_right_ray(int from, int count) {
    uint64_t board = 0ULL;
    int current_index = from - 7;
    for (int i = 0; i < count && current_index >= 0 && current_index % 8 != 0; i++) {
        board |= 1ULL << current_index;
        current_index -= 7;
    }
    return board;
}

uint64_t bit_board_down_left_ray(int from, int count) {
    uint64_t board = 0ULL;
    int current_index = from - 9;
    for (int i = 0; i < count && current_index >= 0 && current_index % 8 != 7; i++) {
        board |= 1ULL << current_index;
        current_index -= 9;
    }
    return board;
}

int min(int n1, int n2) {
    if (n1 <= n2) {
        return n1;
    }
    return n2;
}