#ifndef COMMON
#define COMMON

#define BOARD_SIZE 12
#define EMPTY 0
#define BLACK 1
#define WHITE 2

// option
typedef int OPTION;
#define UP 0
#define DOWN 1
#define LEFT 2
#define RIGHT 3
#define UP_LEFT 4
#define UP_RIGHT 5
#define DOWN_LEFT 6
#define DOWN_RIGHT 7

struct Command
{
  int x;
  int y;
  OPTION option;
};

#endif
