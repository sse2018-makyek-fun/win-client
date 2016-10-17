#include <stdio.h>
#include <stdlib.h>
#include <WinSock2.h>
#include <windows.h>
#include <string.h>
#include <iphlpapi.h>
#include <unistd.h>

#include "common.h"
#include "ai.h"

#define START "START"
#define PLACE "PLACE"
#define READY "READY"
#define TURN  "TURN" 
#define WIN   "WIN"
#define LOSE  "LOSE"

#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720

#define LIST_SIZE 10
#define INFO_X 100
#define INFO_Y 1
#define MESSAGE_X 100
#define MESSAGE_Y 20

struct globalArgs_t {
	char *ip;
	int port;
	BOOL DEBUG;
} globalArgs;

static const char *optString = "a:p:hD";

struct pointer
{
	char str[51];
	int bgColor;
	int fgColor;
	struct pointer *prev;
	struct pointer *next;
};

const char *EMPTY_MESSAGE = "                                                  ";

struct pointer *infoList;
struct pointer *messageList;

char buffer[MAXBYTE] = {0};
char board[BOARD_SIZE][BOARD_SIZE] = {0};
SOCKET sock;
HANDLE hin;
HANDLE hout;
int step = 0;

/*
 * 工具部分
 */
 
BOOL isIp(const char *ip)
{
	int num;
	int flag = TRUE;
	int counter = 0;
	
	memset(buffer, 0, sizeof(buffer));
	strcpy(buffer, ip);
	char *p = strtok(buffer, ".");
	
	while (p && flag)
	{
		num = atoi(p);
		
		if (num >= 0 && num <=255 && (counter++ < 4))
		{
			flag = TRUE;
			p = strtok(NULL, ".");
		}
		else
		{
			flag = FALSE;
			break;
		}
	}
	
	return flag && (counter == 4);
}

BOOL isPort(const int port)
{
	return (port >= 0 && port <= 65535);
}

char *getIp()
{
	PHOSTENT hostinfo;
	char name[255];
	char* ip;
    if(gethostname(name, sizeof(name)) == 0)
    {
        if((hostinfo = gethostbyname(name)) != NULL)
        {
            ip = inet_ntoa (*(struct in_addr *)*hostinfo->h_addr_list);
            return ip;
        }
    }
    return NULL;
} 


/*
 * 数据结构部分
 */
 
void insertStrToList(struct pointer **p, const char *str)
{
	*p = (*p)->prev;
	strcpy((*p)->str, str);
}
 
void initList(struct pointer **p)
{
	int i;
	struct pointer *head, *tail, *tp;
	head = (struct pointer *) malloc(sizeof(struct pointer));
	head->bgColor = 0;
	head->fgColor = 7;
	strcpy(head->str, EMPTY_MESSAGE);
	tail = head;
	
	for (i = 1; i < LIST_SIZE; i++)
	{
		tp = (struct pointer *) malloc(sizeof(struct pointer));
		tp->bgColor = 0;
		tp->fgColor = 7;
		strcpy(tp->str, EMPTY_MESSAGE);
		tp->next = head;
		head->prev = tp;
		head = tp;
	}
	
	head->prev = tail;
	tail->next = head;
	*p = head;
}

void initVars()
{
	initList(&infoList);
	initList(&messageList);
}

/*
 * UI部分 
 */
void setConsoleSize(int width, int height)
{
	system("mode con cols=180 lines=40");
}
 
/* 将光标移动到指定位置 */
void moveCursorTo(const int X, const int Y)
{
	COORD coord;
	coord.X = X;
	coord.Y = Y;
	SetConsoleCursorPosition(hout, coord);
}

/* 设置指定的颜色 */
void setColor(const int bg_color, const int fg_color)
{
	SetConsoleTextAttribute(hout, bg_color * 16 + fg_color);
}

/* 显示光标 */ 
void showConsoleCursor(BOOL showFlag)
{
	CONSOLE_CURSOR_INFO cursorInfo;

	GetConsoleCursorInfo(hout, &cursorInfo);
	cursorInfo.bVisible = showFlag; // set the cursor visibility
	SetConsoleCursorInfo(hout, &cursorInfo);
}

/* 在指定位置显示字符串 */ 
void showStrAt(const struct pointer *p, int x, int y)
{
	moveCursorTo(x, y);
	printf(EMPTY_MESSAGE);
	moveCursorTo(x, y);
	setColor(p->bgColor, p->fgColor);
	printf(p->str);
	setColor(0, 7);
}

void showInfo(const char *info)
{
	insertStrToList(&infoList, info);
	infoList->bgColor = 0;
	infoList->fgColor = 7;
	struct pointer *p = infoList;
	int i;
	for (i = 0; i < LIST_SIZE; i++)
	{
		showStrAt(p, INFO_X, INFO_Y + i);
		p = p->next;
	}
}

void showInfoWithColor(const char *info, int bgColor, int fgColor)
{
	insertStrToList(&infoList, info);
	infoList->bgColor = bgColor;
	infoList->fgColor = fgColor;
	struct pointer *p = infoList;
	int i;
	for (i = 0; i < LIST_SIZE; i++)
	{
		showStrAt(p, INFO_X, INFO_Y + i);
		p = p->next;
	}
}

void showMessage(const char *message)
{
	insertStrToList(&messageList, message);
	messageList->bgColor = 0;
	messageList->fgColor = 7;
	struct pointer *p = messageList;
	int i;
	for (i = 0; i < LIST_SIZE; i++)
	{
		showStrAt(p, MESSAGE_X, MESSAGE_Y + i);
		p = p->next;
	}
}

void resetBoard()
{
	setColor(8, 0);
	moveCursorTo(0, 0); 
	
	/*
	printf("  1   2   3   4   5   6   7   8   9  10  11  12  13  14  15  16  17  18  19\n");
	printf("A┏━┳━┳━┳━┳━┳━┳━┳━┳━┳━┳━┳━┳━┳━┳━┳━┳━┳━┓\n");
	printf(" ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃\n");
	printf("B┣━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━┫\n");
	printf(" ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃\n");
	printf("C┣━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━┫\n");
	printf(" ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃\n");
	printf("D┣━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━┫\n");
	printf(" ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃\n");
	printf("E┣━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━┫\n");
	printf(" ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃\n");
	printf("F┣━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━┫\n");
	printf(" ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃\n");
	printf("G┣━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━┫\n");
	printf(" ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃\n");
	printf("H┣━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━┫\n");
	printf(" ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃\n");
	printf("I┣━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━┫\n");
	printf(" ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃\n");
	printf("J┣━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━┫\n");
	printf(" ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃\n");
	printf("K┣━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━┫\n");
	printf(" ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃\n");
	printf("L┣━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━┫\n");
	printf(" ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃\n");
	printf("M┣━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━┫\n");
	printf(" ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃\n");
	printf("N┣━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━┫\n");
	printf(" ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃\n");
	printf("O┣━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━┫\n");
	printf(" ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃\n");
	printf("P┣━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━┫\n");
	printf(" ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃\n");
	printf("Q┣━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━┫\n");
	printf(" ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃\n");
	printf("R┣━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━┫\n");
	printf(" ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃\n");
	printf("S┗━┻━┻━┻━┻━┻━┻━┻━┻━┻━┻━┻━┻━┻━┻━┻━┻━┻━┛\n");
	*/
	
	printf("  0   1   2   3   4   5   6   7   8   9  10  11  12  13  14\n");
	printf("A┏━┳━┳━┳━┳━┳━┳━┳━┳━┳━┳━┳━┳━┳━┓\n");
	printf(" ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃\n");
	printf("B┣━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━┫\n");
	printf(" ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃\n");
	printf("C┣━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━┫\n");
	printf(" ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃\n");
	printf("D┣━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━┫\n");
	printf(" ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃\n");
	printf("E┣━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━┫\n");
	printf(" ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃\n");
	printf("F┣━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━┫\n");
	printf(" ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃\n");
	printf("G┣━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━┫\n");
	printf(" ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃\n");
	printf("H┣━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━┫\n");
	printf(" ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃\n");
	printf("I┣━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━┫\n");
	printf(" ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃\n");
	printf("J┣━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━┫\n");
	printf(" ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃\n");
	printf("K┣━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━┫\n");
	printf(" ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃\n");
	printf("L┣━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━┫\n");
	printf(" ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃\n");
	printf("M┣━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━┫\n");
	printf(" ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃\n");
	printf("N┣━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━╋━┫\n");
	printf(" ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃\n");
	printf("O┗━┻━┻━┻━┻━┻━┻━┻━┻━┻━┻━┻━┻━┻━┛\n");
	
	
	setColor(0, 7);
}

void initUI()
{
	hin = GetStdHandle(STD_INPUT_HANDLE);
	hout = GetStdHandle(STD_OUTPUT_HANDLE);
	
	showConsoleCursor(FALSE);
	
	setConsoleSize(WINDOW_WIDTH, WINDOW_HEIGHT);
	
	resetBoard();
}

BOOL putChessAt(int x, int y)
{
	if (board[x][y] != EMPTY) return FALSE;
	
	if (step % 2 + 1 == BLACK)
	{
		board[x][y] = BLACK;
		setColor(8, 0);
	}
	else
	{
		board[x][y] = WHITE;
		setColor(8, 15);	
	}
	
	moveCursorTo(4 * x + 1, 2 * y + 1);
	printf("●");
	
	setColor(0, 7);
	++step;
	
	return TRUE;
}


/*
 * Socket部分 
 */
void sendTo(const char *message, SOCKET *sock)
{
	send(*sock, message, strlen(message)+sizeof(char), NULL);
}

void startSock()
{
    //初始化DLL 
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
}

void initSock(const char *ip, const int port)
{
    //创建套接字 
    sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    
    //向服务器发送请求 
    struct sockaddr_in sockAddr;
    memset(&sockAddr, 0, sizeof(sockAddr));  //每个字节都用0填充 
    sockAddr.sin_family = PF_INET;
    sockAddr.sin_addr.s_addr = inet_addr(ip);
    sockAddr.sin_port = htons(port);
    
    memset(buffer, 0, sizeof(buffer));
    sprintf(buffer, "Trying to connect to %s:%d", ip, port);
    showInfo(buffer);
    while (connect(sock, (SOCKADDR*)&sockAddr, sizeof(SOCKADDR)))
    {
    	showInfoWithColor("Connect failed, retry after 5s...\n", 0, FOREGROUND_RED);
    	sleep(5);
	}
	
    showInfoWithColor("Connected\n", 0, FOREGROUND_GREEN);
}

void closeSock()
{
    //关闭套接字 
    closesocket(sock);
    
    //终止使用DLL 
    WSACleanup();
}


void ready()
{
	/* TODO: 拆分成人工下棋和AI下棋 */ 
	/* 从CMD里直接点击位置 */ 
	INPUT_RECORD ir[128];
	DWORD nRead;
	COORD xy;
	UINT i;
	SetConsoleMode(hin, ENABLE_MOUSE_INPUT | ENABLE_EXTENDED_FLAGS);
	
	if (TRUE == globalArgs.DEBUG) {
		/*
		 * 人工部分 
		 */
		while (TRUE)
		{
			ReadConsoleInput(hin, ir, 128, &nRead);
			for (i = 0; i < nRead; i++)
			{
				if (MOUSE_EVENT == ir[i].EventType && FROM_LEFT_1ST_BUTTON_PRESSED == ir[i].Event.MouseEvent.dwButtonState)
				{
					int rawX = ir[i].Event.MouseEvent.dwMousePosition.X;
					int rawY = ir[i].Event.MouseEvent.dwMousePosition.Y;
					
					if (rawX % 4 == 0 || rawX % 4 == 3 || rawY % 2 == 0) continue;
					
					int x = rawX / 4;
					int y = rawY / 2;
					
					if (x >= 0 && x < BOARD_SIZE && y >= 0 && y < BOARD_SIZE && putChessAt(x, y))
					{
				    	memset(buffer, 0, sizeof(buffer));
				    	sprintf(buffer, "%d %d\n", x, y);
				    	sendTo(buffer, &sock);
						return;
					}
				}
			}
		}
	}
	else
	{
		/*
		 * AI部分 
		 */
		 
		struct Position pos = ai(board);
		int x = pos.x;
		int y = pos.y;
		
		if (x >= 0 && x < BOARD_SIZE && y >= 0 && y < BOARD_SIZE && putChessAt(x, y))
		{
		   	memset(buffer, 0, sizeof(buffer));
		   	sprintf(buffer, "%d %d\n", x, y);
		   	sendTo(buffer, &sock);
			return;
		}
	}
	
}

void turn(int x, int y)
{
	putChessAt(x, y);
}

void win()
{
	showInfo("You win!\n");
}

void lose()
{
	showInfo("You Lose!\n");
}

void work()
{
	while (TRUE)
    {
    	memset(buffer, 0, sizeof(buffer));
    	//接收服务器传回的数据 
    	recv(sock, buffer, MAXBYTE, NULL);
    	
    	//输出接收到的数据 
    	showMessage(buffer);
    	
    	if (strstr(buffer, START))
    	{
    		
		}
		else if (strstr(buffer, PLACE))
		{
			char tmp[MAXBYTE] = {0};
			int x,  y;
			sscanf(buffer, "%s %d %d", tmp, &x, &y);
			turn(x, y);
		}
    	else if (strstr(buffer, READY))
    	{
    		ready();
		}
		else if (strstr(buffer, TURN))
		{
			char tmp[MAXBYTE] = {0};
			int x,  y;
			sscanf(buffer, "%s %d %d", tmp, &x, &y);
			turn(x, y);
		}
		else if (strstr(buffer, WIN))
		{
			win();
		}
		else if (strstr(buffer, LOSE))
		{
			lose();
		}
	}
}

void display_usage(char *exe)
{
	printf("Usage: %s [OPTIONS] \n", exe);
	printf("  -a address        Server address\n");
	printf("  -p port           Server port\n");
	printf("  -D                Debug mode. When set, the user will manually play the chess.\n");
}

void initArgs(int argc, char *argv[])
{
	int opt = 0;
	globalArgs.DEBUG = FALSE;
	globalArgs.ip = getIp();
	globalArgs.port = 23333;
	
	opt = getopt(argc, argv, optString);
	while (opt != -1)
	{
		switch (opt)
		{
			case 'a':
				globalArgs.ip = optarg;
				break;
			case 'p':
				globalArgs.port = atoi(optarg);
				break;
			case 'D':
				globalArgs.DEBUG = TRUE;
				break;
			case 'h':
				display_usage(argv[0]);
				exit(0);
				break;
			default:
				// Illegal!
				break;
		}
		
		opt = getopt(argc, argv, optString);
	}
}

int main(int argc, char *argv[])
{
	startSock();
	
	initArgs(argc, argv);
	initVars();
	initUI();
	
	initSock(globalArgs.ip, globalArgs.port);
	
	work();
	closeSock();

    return 0;
}
