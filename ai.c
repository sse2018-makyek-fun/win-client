#include "ai.h"

/* START */

/* You can define your own struct or variable here */

/* me: BLACK or WHITE(defined in common.h */
struct Position ai(const char board[BOARD_SIZE][BOARD_SIZE], int me)
{
    /*
     * TODO: Write your own ai here!
     * Here is a simple ai which just put chess at empty position!
     */
    int i, j;
    struct Position preferedPos;
    
    for (i = 0; i < BOARD_SIZE; i++)
        for (j = 0; j < BOARD_SIZE; j++)
        {
            if (EMPTY == board[i][j])
            {
                preferedPos.x = i;
                preferedPos.y = j;
                return preferedPos;
            }
        }
    
    
    return preferedPos;
}

/* END */