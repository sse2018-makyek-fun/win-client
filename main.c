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

char buffer[MAXBYTE] = {0};
SOCKET sock;

/*
 * UI部分 
 */
 
void removeScrollBar()
{
    HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO info;
    GetConsoleScreenBufferInfo(handle, &info);
    COORD new_size = 
    {
        info.srWindow.Right - info.srWindow.Left + 1,
        info.srWindow.Bottom - info.srWindow.Top + 1
    };
    SetConsoleScreenBufferSize(handle, new_size);
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
void moveCursorTo(HANDLE hout, const int X, const int Y)
{
	COORD coord;
	coord.X = X;
	coord.Y = Y;
	SetConsoleCursorPosition(hout, coord);
}

/* 设置指定的颜色 */
void setColor(HANDLE hout, const int bg_color, const int fg_color)
{
	SetConsoleTextAttribute(hout, bg_color * 16 + fg_color);
}

void initUI()
{
	setConsoleSize(WINDOW_WIDTH, WINDOW_HEIGHT);
	HANDLE hout = GetStdHandle(STD_OUTPUT_HANDLE); 
	
	setColor(hout, 8, 0); 
	
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
	
	
	setColor(hout, 0, 7);
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
    	printf("Connect failed, retry after 5s...\n");
    	sleep(5);
	}
    
    printf("Connected");
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
	
}

void turn(int x, int y)
{
	
}

void win()
{
	
}

void lose()
{
	
}

void work()
{
	initUI();
	while (TRUE)
    {
    	memset(buffer, 0, sizeof(buffer));
    	//接收服务器传回的数据 
    	recv(sock, buffer, MAXBYTE, NULL);
    	//输出接收到的数据 
    	printf("Message form server: %s", buffer);
    	
    	if (strstr(buffer, READY))
    	{
    		ready();
		}
		else if (strstr(buffer, TURN))
		{
			//TODO: parse
			char tmp[MAXBYTE] = {0};
			int row,  col;
			sscanf(buffer, "%s %d %d", tmp, row, col);
			turn(row, col);
		}
		else if (strstr(buffer, WIN))
		{
			win();
		}
		else if (strstr(buffer, LOSE))
		{
			lose();
		}
    	
    	int row, col;
    	scanf("%d %d", &row, &col);
    	
    	memset(buffer, 0, sizeof(buffer));
    	sprintf(buffer, "%d %d\n", row, col);
    	
    	sendTo("buffer", &sock);
	}
}

int main(){
	
	initUI();
	
	initSock();
	work();
	closeSock();

    return 0;
}
