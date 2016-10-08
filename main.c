#include <stdio.h>
#include <stdlib.h>
#include <WinSock2.h>
#include <windows.h>
#include <unistd.h>

#define READY "READY"
#define TURN  "TURN" 
#define WIN   "WIN"
#define LOSE  "LOSE"

#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720

#define BOARD_SIZE 19
#define BLACK 0
#define WHITE 1

#define INFO_X 100
#define INFO_Y 1
#define MESSAGE_X 100
#define MESSAGE_Y 20

const char *EMPTY_MESSAGE = "                                                  ";

char buffer[MAXBYTE] = {0};
char board[BOARD_SIZE][BOARD_SIZE] = {0};
SOCKET sock;
HANDLE hin;
HANDLE hout;
int step = 0;

/*
 * UI部分 
 */
 
void removeScrollBar()
{
    CONSOLE_SCREEN_BUFFER_INFO info;
    GetConsoleScreenBufferInfo(hout, &info);
    COORD new_size = 
    {
        info.srWindow.Right - info.srWindow.Left + 1,
        info.srWindow.Bottom - info.srWindow.Top + 1
    };
    SetConsoleScreenBufferSize(hout, new_size);
}
 
void setConsoleSize(int width, int height)
{
    HWND console = GetConsoleWindow();
    RECT r;
    GetWindowRect(console, &r);

    MoveWindow(console, r.left, r.top, width, height, TRUE);
    removeScrollBar();
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
void showStrAt(const char *str, int x, int y)
{
	moveCursorTo(x, y);
	printf(EMPTY_MESSAGE);
	moveCursorTo(x, y);
	printf(str);
}

void showInfo(const char *info)
{
	showStrAt(info, INFO_X, INFO_Y);
}

void showMessage(const char *message)
{
	showStrAt(message, MESSAGE_X, MESSAGE_Y);
}

void initUI()
{
	hin = GetStdHandle(STD_INPUT_HANDLE);
	hout = GetStdHandle(STD_OUTPUT_HANDLE);
	
	showConsoleCursor(FALSE);
	
	setConsoleSize(WINDOW_WIDTH, WINDOW_HEIGHT);
	
	setColor(8, 0);
	moveCursorTo(0, 0); 
	
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
	
	
	setColor(0, 7);
}

BOOL putChessAt(int x, int y)
{
	if (board[x][y] != 0) return FALSE;
	
	if (step % 2 == BLACK)
	{
		setColor(8, 0);
	}
	else
	{
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

void initSock()
{
    //初始化DLL 
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
    
    //创建套接字 
    sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    
    //向服务器发送请求 
    struct sockaddr_in sockAddr;
    memset(&sockAddr, 0, sizeof(sockAddr));  //每个字节都用0填充 
    sockAddr.sin_family = PF_INET;
    sockAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    sockAddr.sin_port = htons(23333);
    
    while (connect(sock, (SOCKADDR*)&sockAddr, sizeof(SOCKADDR)))
    {
    	showInfo("Connect failed, retry after 5s...\n");
    	sleep(5);
	}
	
    showInfo("Connected\n");
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
	/* 从CMD里直接点击位置 */ 
	INPUT_RECORD ir[128];
	DWORD nRead;
	COORD xy;
	UINT i;
	SetConsoleMode(hin, ENABLE_MOUSE_INPUT | ENABLE_EXTENDED_FLAGS);
	
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
    	
    	if (strstr(buffer, READY))
    	{
    		ready();
		}
		else if (strstr(buffer, TURN))
		{
			//TODO: parse
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

int main(){
	
	initUI();
	initSock();
	work();
	closeSock();

    return 0;
}
