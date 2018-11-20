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
 * ��Ĵ��뿪ʼ
 */
 
/* 
 * You can define your own struct and variable here
 * ����������ﶨ�����Լ��Ľṹ��ͱ���
 */
 

/*
 * ������������ʼ�����AI
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
 * �ֵ������ӡ�
 * ������0��ʾ�հף�1��ʾ���壬2��ʾ����
 * me��ʾ�������������(1��2) 
 * ����Ҫ����һ���ṹ��Position����x���Ժ�y������������Ҫ���ӵ�λ�á� 
 */
struct Command aiTurn(const char board[BOARD_SIZE][BOARD_SIZE], int me) {
    /*
     * TODO��������д�����AI�� 
     * ������һ��ʾ��AI����ֻ��Ѱ�ҵ�һ�����µ�λ�ý������ӡ� 
     */
    struct Command preferedPos = findValidPos(board, me);

	return preferedPos;
}

/*
 * ��Ĵ������ 
 */
