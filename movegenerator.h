#ifndef MOEVGENERATOR_H
#define MOEVGENERATOR_H

#include "attacktable.h"

struct Board;


Move* get_legal_moves(Board* board, AttackTable* attack_table);


#endif