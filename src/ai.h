#ifndef AI
#define AI

#include "common.h"

void initAI();

struct Position ai(const char board[BOARD_SIZE][BOARD_SIZE], int me);

#endif
