#include <stdio.h>
#include <stdlib.h>
#include <WinSock2.h>
#include <windows.h>
#include <string.h>
#include <iphlpapi.h>
#include <unistd.h>

#include "common.h"
#include "utils.h"
#include "ai.h"

#define START "START"
#define PLACE "PLACE"
#define TURN  "TURN"
#define END   "END"

#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720

#define LIST_SIZE 10
#define INFO_X 100
#define INFO_Y 1
#define MESSAGE_X 100
#define MESSAGE_Y 20

const int DI[8][2] = { {-1, 0}, {1, 0}, {0, -1}, {0, 1}, {-1, -1}, {-1, 1}, {1, -1}, {1, 1} };

struct globalArgs_t {
    char *ip;
    int port;
    BOOL DEBUG;
} globalArgs;

static const char *optString = "a:p:hD";

//Message and info pointer
struct pointer {
    char str[51];
    int bgColor;
    int fgColor;
    struct pointer *prev;
    struct pointer *next;
};

//Replay pointer
struct rpointer {
    int x;
    int y;
    struct rpointer *prev;
    struct rpointer *next;
};

const char *EMPTY_MESSAGE = "                                                  ";

struct pointer *infoList;
struct pointer *messageList;
struct rpointer *replayList;

char buffer[MAXBYTE] = {0};
char board[BOARD_SIZE][BOARD_SIZE] = {0};
SOCKET sock;
HANDLE hin;
HANDLE hout;
int lastX, lastY, lastOption;
int meFlag, otherFlag;

/*
 * Ip Utils
 */

BOOL isIp(const char *ip)
{
    int num;
    int flag = TRUE;
    int counter = 0;

    memset(buffer, 0, sizeof(buffer));
    strcpy(buffer, ip);
    char *p = strtok(buffer, ".");

    while (p && flag) {
        num = atoi(p);

        if (num >= 0 && num <=255 && (counter++ < 4)) {
            flag = TRUE;
            p = strtok(NULL, ".");
        } else {
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
    if(gethostname(name, sizeof(name)) == 0) {
        if((hostinfo = gethostbyname(name)) != NULL) {
            ip = inet_ntoa (*(struct in_addr *)*hostinfo->h_addr_list);
            return ip;
        }
    }
    return NULL;
}


/*
 * List Utils
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

    for (i = 1; i < LIST_SIZE; i++) {
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
 * UI Utils
 */
void setConsoleSize(int width, int height)
{
    system("mode con cols=180 lines=42");
}

/* Move cursor to specified position in the console */
void moveCursorTo(const int X, const int Y)
{
    COORD coord;
    coord.X = X;
    coord.Y = Y;
    SetConsoleCursorPosition(hout, coord);
}

/*
 * Set background color and forground color
 * See http://baike.baidu.com/item/SetConsoleTextAttribute
 */
void setColor(const int bg_color, const int fg_color)
{
    SetConsoleTextAttribute(hout, bg_color * 16 + fg_color);
}

/* Show cursor */
void showConsoleCursor(BOOL showFlag)
{
    CONSOLE_CURSOR_INFO cursorInfo;

    GetConsoleCursorInfo(hout, &cursorInfo);
    cursorInfo.bVisible = showFlag; // set the cursor visibility
    SetConsoleCursorInfo(hout, &cursorInfo);
}

/* Show char at specified position in the console */
void showStrAt(const struct pointer *p, int x, int y)
{
    moveCursorTo(x, y);
    printf(EMPTY_MESSAGE);
    moveCursorTo(x, y);
    setColor(p->bgColor, p->fgColor);
    printf(p->str);
    setColor(0, 7);
}

/* Show colored info(Top-right) */
void showInfoWithColor(const char *info, int bgColor, int fgColor)
{
    insertStrToList(&infoList, info);
    infoList->bgColor = bgColor;
    infoList->fgColor = fgColor;
    struct pointer *p = infoList;
    int i;
    for (i = 0; i < LIST_SIZE; i++) {
        showStrAt(p, INFO_X, INFO_Y + i);
        p = p->next;
    }
}

/* Show info(Top-right) */
void showInfo(const char *info)
{
	showInfoWithColor(info, 0, 7);
}

/* Show colored message(Bottom-right)*/
void showMessageWithColor(const char *message, int bgColor, int fgColor)
{
    insertStrToList(&messageList, message);
    messageList->bgColor = bgColor;
    messageList->fgColor = fgColor;
    struct pointer *p = messageList;
    int i;
    for (i = 0; i < LIST_SIZE; i++) {
        showStrAt(p, MESSAGE_X, MESSAGE_Y + i);
        p = p->next;
    }
}

/* Show message(Bottom-right)*/
void showMessage(const char *message)
{
	showMessageWithColor(message, 0, 7);
}



/* Reset board */
void resetBoard()
{
    setColor(8, 0);
    moveCursorTo(0, 0);

    printf("    A   B   C   D   E   F   G   H   I   J   K   L    \n");
    printf("  ©³©¥©×©¥©×©¥©×©¥©×©¥©×©¥©×©¥©×©¥©×©¥©×©¥©×©¥©×©¥©· \n");
    printf(" 0©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§ \n");
    printf("  ©Ç©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©Ï \n");
    printf(" 1©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§ \n");
    printf("  ©Ç©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©Ï \n");
    printf(" 2©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§ \n");
    printf("  ©Ç©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©Ï \n");
    printf(" 3©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§ \n");
    printf("  ©Ç©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©Ï \n");
    printf(" 4©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§ \n");
    printf("  ©Ç©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©Ï \n");
    printf(" 5©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§ \n");
    printf("  ©Ç©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©Ï \n");
    printf(" 6©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§ \n");
    printf("  ©Ç©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©Ï \n");
    printf(" 7©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§ \n");
    printf("  ©Ç©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©Ï \n");
    printf(" 8©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§ \n");
    printf("  ©Ç©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©Ï \n");
    printf(" 9©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§ \n");
    printf("  ©Ç©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©Ï \n");
    printf("10©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§ \n");
    printf("  ©Ç©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©Ï \n");
    printf("11©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§ \n");
    printf("  ©»©¥©ß©¥©ß©¥©ß©¥©ß©¥©ß©¥©ß©¥©ß©¥©ß©¥©ß©¥©ß©¥©ß©¥©¿ \n");

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

BOOL inBoard(int x, int y)
{
    return x >= 0 && y >= 0 && x < BOARD_SIZE && y < BOARD_SIZE;
}

void drawChess(int x, int y, char* ch)
{
	moveCursorTo(4 * y + 4, 2 * x + 2);
	printf(ch);
	return;
}

BOOL moveChess(int x, int y, int option, int meFlag)
{
	int otherFlag = 3 - meFlag;
	int i;
	
	if (option < UP || option > DOWN_RIGHT ) return FALSE;
	
	int nextx = x + DI[option][0];
	int nexty = y + DI[option][1];

    if (!inBoard(x, y) || !inBoard(nextx, nexty) || board[x][y] != meFlag && board[nextx][nexty] != EMPTY)
    {
        return FALSE;
    }

    if (meFlag == BLACK) {
        setColor(8, 0);
    } else {
        setColor(8, 15);
    }
    
    board[x][y] = EMPTY;
    drawChess(x, y, " ");
    board[nextx][nexty] = meFlag;
    drawChess(nextx, nexty, "¡ñ");
    
    // Yak
    if (inBoard(nextx - 1, nexty - 1) && inBoard(nextx + 1, nexty + 1) && board[nextx - 1][nexty - 1] == otherFlag && board[nextx + 1][nexty + 1] == otherFlag)
    {
        board[nextx - 1][nexty - 1] = board[nextx + 1][nexty + 1] = meFlag;
		drawChess(nextx - 1, nexty - 1, "¡ñ");
		drawChess(nextx + 1, nexty + 1, "¡ñ");
    }
    if (inBoard(nextx + 1, nexty - 1) && inBoard(nextx - 1, nexty + 1) && board[nextx + 1][nexty - 1] == otherFlag && board[nextx - 1][nexty + 1] == otherFlag)
    {
        board[nextx + 1][nexty - 1] = board[nextx - 1][nexty + 1] = meFlag;
		drawChess(nextx + 1, nexty - 1, "¡ñ");
		drawChess(nextx - 1, nexty + 1, "¡ñ");
    }
	if (nexty - 1 >= 0 && nexty + 1 < BOARD_SIZE && board[nextx][nexty - 1] == otherFlag && board[nextx][nexty + 1] == otherFlag)
	{
		board[nextx][nexty - 1] = board[nextx][nexty + 1] = meFlag;
		drawChess(nextx, nexty - 1, "¡ñ");
		drawChess(nextx, nexty + 1, "¡ñ");
	}
	if (nextx - 1 >= 0 && nextx + 1 < BOARD_SIZE && board[nextx - 1][nexty] == otherFlag && board[nextx + 1][nexty] == otherFlag)
	{
		board[nextx - 1][nexty] = board[nextx + 1][nexty] = meFlag;
		drawChess(nextx - 1, nexty, "¡ñ");
		drawChess(nextx + 1, nexty, "¡ñ");
	}
    
    // Mak
    for (i = 0; i < 8; i++)
    {
		if (inBoard(nextx + 2 * DI[i][0], nexty + 2 * DI[i][1]) &&
			board[nextx + DI[i][0]][nexty + DI[i][1]] == otherFlag && board[nextx + 2 * DI[i][0]][nexty + 2 * DI[i][1]] == meFlag)
		{
			board[nextx + DI[i][0]][nexty + DI[i][1]] = meFlag;
			drawChess(nextx + DI[i][0], nexty + DI[i][1], "¡ñ");
		}
	}

    setColor(0, 7);

    return TRUE;
}


/*
 * Socket Utils
 */
void sendTo(const char *message, SOCKET *sock)
{
    send(*sock, message, strlen(message)+sizeof(char), 0);
    Sleep(100);
}

void startSock()
{
    //Init socket DLL
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
    initSocketBuffer();
}

void initSock(const char *ip, const int port)
{
    //Open socket
    sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

    //Connect to socket
    struct sockaddr_in sockAddr;
    memset(&sockAddr, 0, sizeof(sockAddr));
    sockAddr.sin_family = PF_INET;
    sockAddr.sin_addr.s_addr = inet_addr(ip);
    sockAddr.sin_port = htons(port);

    memset(buffer, 0, sizeof(buffer));
    sprintf(buffer, "Trying to connect to %s:%d", ip, port);
    showInfo(buffer);
    while (connect(sock, (SOCKADDR*)&sockAddr, sizeof(SOCKADDR))) {
        showInfoWithColor("Connect failed, retry after 5s...\n", 0, FOREGROUND_RED);
        sleep(5);
    }

    showInfoWithColor("Connected\n", 0, FOREGROUND_GREEN);
}

void closeSock()
{
    //Close socket
    closesocket(sock);

    //Close socket DLL
    WSACleanup();
}

void start()
{
	int i;
    memset(board, 0, sizeof(board));
    
    for (i = 0; i < 3; i++)
	{
        setColor(8, 15);
    	board[2][2+i] = board[6][6+i] = WHITE;
    	drawChess(2, 2+i, "¡ñ");
    	drawChess(6, 6+i, "¡ñ");
    	setColor(8, 0);
    	board[5][3+i] = board[9][7+i] = BLACK;
    	drawChess(5, 3+i, "¡ñ");
    	drawChess(9, 7+i, "¡ñ");
	}
	for (i = 0; i < 2; i++)
	{
        setColor(8, 15);
    	board[8+i][2] = WHITE;
    	drawChess(8+i, 2, "¡ñ");
    	setColor(8, 0);
    	board[2+i][9] = BLACK;
    	drawChess(2+i, 9, "¡ñ");
	}

	setColor(0, 7);
	
	if (meFlag == BLACK)
		showInfoWithColor("You are BLACK!", 0, FOREGROUND_GREEN);
	else
		showInfoWithColor("You are WHITE!", 0, FOREGROUND_GREEN);
    
    
    lastX = 0;
    lastY = 0;
    lastOption = 0;
    initAI(meFlag);
}

void turn()
{
    INPUT_RECORD ir[128];
    DWORD nRead;
    COORD xy;
    UINT i, j;
    SetConsoleMode(hin, ENABLE_MOUSE_INPUT | ENABLE_EXTENDED_FLAGS);

    if (TRUE == globalArgs.DEBUG) {
        /*
         * For debug
         */
        showMessageWithColor("Please move your chess!\n", 0, FOREGROUND_GREEN);
        struct Command command = {-1, -1, 0};
        while (TRUE) {
            ReadConsoleInput(hin, ir, 128, &nRead);
            for (i = 0; i < nRead; i++) {
                if (MOUSE_EVENT == ir[i].EventType && FROM_LEFT_1ST_BUTTON_PRESSED == ir[i].Event.MouseEvent.dwButtonState) {
                    int rawX = ir[i].Event.MouseEvent.dwMousePosition.X;
                    int rawY = ir[i].Event.MouseEvent.dwMousePosition.Y;
                    
                    if (rawX % 4 != 0 || rawY % 2 != 0) continue;

                    int y = rawX / 4 - 1;
                    int x = rawY / 2 - 1;

                    if (!inBoard(x, y)) continue;
                    
                    if (board[x][y] == meFlag)
                    {
	                    memset(buffer, 0, sizeof(buffer));
	                    sprintf(buffer, "Choose x = %d, y = %d\n", x, y);
                    	showMessage(buffer);
                		command.x = x;
                		command.y = y;
                	}
                	else if (board[x][y] == EMPTY)
                	{
                		if (inBoard(command.x, command.y))
                		{
                			int dx = x - command.x, dy = y - command.y;
                			for (j = 0; j < 8; j++)
                			{
                				if (dx == DI[j][0] && dy == DI[j][1])
                				{
									if (moveChess(command.x, command.y, j, meFlag)) {
										memset(buffer, 0, sizeof(buffer));
				                        sprintf(buffer, "%d %d %d\n", command.x, command.y, j);
				                        showMessage(buffer);
				                        sendTo(buffer, &sock);
				                        return;
				                    }
								}
							}
						}
					}
                }
            }
        }
    } else {
        /*
         * For AI
         */
		
		while (TRUE)
		{
	        struct Command command = aiTurn((const char (*)[BOARD_SIZE])board, meFlag);
	        int x = command.x;
	        int y = command.y;
	        int option = command.option;
	        if (moveChess(x, y, option, meFlag)) {
	            memset(buffer, 0, sizeof(buffer));
	            sprintf(buffer, "%d %d %d\n", x, y, option);
	            showMessage(buffer);
	            sendTo(buffer, &sock);
	            return;
	        }
    	}
    }

}

void place(int x, int y, int option)
{
    moveChess(x, y, option, otherFlag);
}

void draw()
{
    showInfo("Draw!\n");
}

void win()
{
    showInfoWithColor("You win!\n", 0, FOREGROUND_GREEN);
}

void lose()
{
    showInfoWithColor("You Lose!\n", 0, FOREGROUND_RED);
}

void end(int result)
{
    if (result == 0)
        draw();
    else if (result == meFlag)
        win();
    else
        lose();
}

void loop()
{
    while (TRUE) {
        memset(buffer, 0, sizeof(buffer));
        //Receive message from server
        recv(sock, buffer, MAXBYTE, 0);

        addToSocketBuffer(buffer);

        while (hasCommand('\n')) {
            showMessage(socketArg);

            if (strstr(socketArg, START)) {
                char tmp[MAXBYTE] = {0};
                sscanf(socketArg, "%s %d", tmp, &meFlag);
                otherFlag = 3 - meFlag;
                start();
            } else if (strstr(socketArg, PLACE)) {
                char tmp[MAXBYTE] = {0};
                int x, y, option;
                sscanf(socketArg, "%s %d %d %d", tmp, &x, &y, &option);
                place(x, y, option);
            } else if (strstr(socketArg, TURN)) {
                turn();
            } else if (strstr(socketArg, END)) {
                char tmp[MAXBYTE] = {0};
                int result;
                sscanf(socketArg, "%s %d", tmp, &result);
                end(result);
            }
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
    while (opt != -1) {
        switch (opt) {
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
                printf("Illegal arguments!\n");
                display_usage(argv[0]);
                exit(0);
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
        
	loop();

    closeSock();

    return 0;
}
