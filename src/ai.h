#ifndef AI
#define AI

#include "common.h"

void initAI(int me);
struct Command aiTurn(const char board[BOARD_SIZE][BOARD_SIZE], int me);

#endif
