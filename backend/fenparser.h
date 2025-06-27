#ifndef FENPARSER_H
#define FENPARSER_H

typedef struct board Board;

Board* fen_to_board(char* fen, int size);

char* board_to_fen(Board* board);

#endif