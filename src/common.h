#ifndef COMMON
#define COMMON

#define BOARD_SIZE 8
#define EMPTY 0
#define BLACK 1
#define WHITE 2

// option
typedef int OPTION;
#define UP 0
#define DOWN 1
#define LEFT 2
#define RIGHT 3

struct Command
{
  int x;
  int y;
  OPTION option;
};

#endif
