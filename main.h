#pragma once
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <string.h>
#include <direct.h>
#include <limits.h>
#include <lmcons.h>
#include <ctype.h>
#include <signal.h>
#include "commands.c"

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

typedef void (*CommandFunc)(char*, char**, int*);

typedef struct Command {
    const char* name;
    CommandFunc func;
} Command;

void echo(char *inputCommand, char **args, int *argc);
void ls(char *inputCommand, char **args, int *argc);
void cd(char *inputCommand, char **args, int *argc);
void clear(char*, char**, int*);
void exitShell(char*, char**, int*);
void touch(char *inputCommand, char **args, int *argc);
void makedir(char *inputCommand, char **args, int *argc);
void rm(char *inputCommand, char **args, int *argc);
int getCursorY();
int getFilesCount(char *path);
int hasAlphanumeric(char *arg);
char *trim(char* str);
int *getFilesLen(char *path, int fileCount);
void printHistory(char **history, int *historyCount);
char username[UNLEN + 1];
DWORD username_len = sizeof(username);

Command commands[] = {
    { "echo", echo },
    { "ls", ls },
    { "cd", cd },
    { "clear", clear },
    { "exit", exitShell },
    { "touch", touch },
    { "mkdir", makedir },
    { "rm", rm }
};

const int commandCount = sizeof(commands) / sizeof(Command);