#ifndef MOEVGENERATOR_H
#define MOEVGENERATOR_H

#include "attacktable.h"
#include "move.h"
#include "board.h"


Move* get_legal_moves(Board* board, AttackTable* attack_table);


#endif