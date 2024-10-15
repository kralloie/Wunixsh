#ifndef MAIN_H
#define MAIN_H

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
#include <math.h>

#define SHELL_PREFIX "$ "
#define MAX_INPUT 1024
#define MAX_ARGS 100
#define RUNNING 1
#define TAB_KEY 9
#define ENTER_KEY 13
#define BACKSPACE 8
#define SPACEBAR 32
#define UP_ARROW 0x48
#define DOWN_ARROW 0x50
#define RIGHT_ARROW 0x4D
#define LEFT_ARROW 0x4B
#define MAX_HISTORY 100
#define SHTDN_REASON_MAJOR_SOFTWARE 0x00030000

typedef void (*CommandFunc)(char*, char**, int*);

typedef struct Command {
    const char* name;
    CommandFunc func;
} Command;

extern char username[];
extern DWORD username_len;
extern char *history[];
extern int historyCount;

void echo(char *inputCommand, char **args, int *argc);
void ls(char *inputCommand, char **args, int *argc);
void cd(char *inputCommand, char **args, int *argc);
void clear(char*, char**, int*);
void exitShell(char*, char**, int*);
void touch(char *inputCommand, char **args, int *argc);
void makedir(char *inputCommand, char **args, int *argc);
void rm(char *inputCommand, char **args, int *argc);
void cat(char *inputCommand, char **args, int *argc);
void historyCommand(char *inputCommand, char **args, int *argc);
void cp(char *inputCommand, char **args, int *argc);
void mv(char *inputCommand, char **args, int *argc);
void pwd(char*, char**, int*);
void shtdwn(char*, char**, int*);
void rstrt(char*, char**, int*);
void df(char*, char**, int*);
void ipconfig(char*, char**, int*);

int getCursorY();
int getCursorX();
int getFilesCount(char *path);
int *getFilesLen(char *path, int fileCount);
char **getPathAndFilename(char* path);
char **getFileNames(char *path);
char *strToLower(char *str, int length);
char *getQuotatedName(char *str);
int hasAlphanumeric(char *arg);

void prompt(HANDLE, SYSTEMTIME, char*, char*, char*);
void executeCommand(char *input);

extern int historyCount;
extern const Command commands[];
extern int commandCount;

#endif