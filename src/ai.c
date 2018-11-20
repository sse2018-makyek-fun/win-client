#include "ai.h"

// bool
typedef int BOOL;
#define TRUE 1
#define FALSE 0

const int DIR[8][2] = { {-1, 0}, {1, 0}, {0, -1}, {0, 1}, {-1, -1}, {-1, 1}, {1, -1}, {1, 1} };

BOOL isInBound(int x, int y) {
  return x >= 0 && x < BOARD_SIZE && y >= 0 && y < BOARD_SIZE;
}

/*
 * YOUR CODE BEGIN
 * 你的代码开始
 */
 
/* 
 * You can define your own struct and variable here
 * 你可以在这里定义你自己的结构体和变量
 */
 

/*
 * 你可以在这里初始化你的AI
 */
void initAI(int me)
{

}

struct Command findValidPos(const char board[BOARD_SIZE][BOARD_SIZE], int flag) {
  struct Command command = {0, 0, 0};
  int k, x, y;
  for (k = 0; k < 8; k++) {
    const int* delta = DIR[k];
    for (x = 0; x < BOARD_SIZE; x++) {
      for (y = 0; y < BOARD_SIZE; y++) {
        if (board[x][y] != flag) {
          continue;
        }
        int new_x = x + delta[0];
        int new_y = y + delta[1];
        if (isInBound(new_x, new_y) && board[new_x][new_y] == EMPTY) {
          command.x = x;
          command.y = y;
          command.option = k;
          return command;
        }
      }
    }
  }
  return command;
}


/*
 * 轮到你落子。
 * 棋盘上0表示空白，1表示黑棋，2表示白旗
 * me表示你所代表的棋子(1或2) 
 * 你需要返回一个结构体Position，在x属性和y属性填上你想要落子的位置。 
 */
struct Command aiTurn(const char board[BOARD_SIZE][BOARD_SIZE], int me) {
    /*
     * TODO：在这里写下你的AI。 
     * 这里有一个示例AI，它只会寻找第一个可下的位置进行落子。 
     */
    struct Command preferedPos = findValidPos(board, me);

	return preferedPos;
}

/*
 * 你的代码结束 
 */
