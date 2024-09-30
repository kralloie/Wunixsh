#pragma once
#include <windows.h>

#define SHELL_PREFIX "~ "
#define MAX_INPUT 1024
#define MAX_ARGS 100
#define RUNNING 1
#define TAB_KEY 9
#define ENTER_KEY 13
#define BACKSPACE 8
#define MAX_INPUT 1024
#define SPACEBAR 32
#define UP_ARROW 0x48
#define DOWN_ARROW 0x50
#define RIGHT_ARROW 0x4D
#define LEFT_ARROW 0x4B
#define MAX_HISTORY 100

int getCursorY() {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);;
    CONSOLE_SCREEN_BUFFER_INFO csbi;

    if(GetConsoleScreenBufferInfo(hConsole, &csbi)) {
        return csbi.dwCursorPosition.Y + 1;
    } else {
        return 0;
    }
}

void printHistory(char** history, int *historyCount) {
    printf("\n");
    printf("---------History---------\n");
    printf("Last Position: %s\n", history[*historyCount - 1]);
    printf("History Count: %d\n", *historyCount);
    for(int i = 0; i < *historyCount; i++ ) {
        printf("%s - ", history[i]);
    }
    printf("\n");
    printf("---------History---------\n");
}
