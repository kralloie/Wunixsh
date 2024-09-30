#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <direct.h>
#include <conio.h>
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

const Command commands[] = {
    { "echo", echo },
    { "ls", ls },
    { "cd", cd },
    { "clear", clear },
    { "exit", exitShell }
};

int getCursorY() {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);;
    CONSOLE_SCREEN_BUFFER_INFO csbi;

    if(GetConsoleScreenBufferInfo(hConsole, &csbi)) {
        return csbi.dwCursorPosition.Y + 1;
    } else {
        return 0;
    }
}

int main() {
    char *history[MAX_HISTORY];
    int historyCount = 0;
    
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    char username[UNLEN + 1];
    DWORD username_len = sizeof(username);
    GetUserName(username, &username_len);

    int commandCount = sizeof(commands) / sizeof(Command);

    while (RUNNING) {
        SYSTEMTIME time;
        GetSystemTime(&time);
        int historyIndex = 0;

        char cwd[MAX_PATH];
        int validCommand = 0;
        if(_getcwd(cwd, sizeof(cwd)) == NULL) {
            printf("Unexpected Error!");
        }

        SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_BLUE);
        printf("\033[1m[%s - %02d:%02d:%02d] - \033[0m", username, time.wHour, time.wMinute, time.wSecond);
        SetConsoleTextAttribute(hConsole, FOREGROUND_GREEN | FOREGROUND_BLUE);
        printf("\033[1m%s\033[0m\n", cwd);
        SetConsoleTextAttribute(hConsole, 7);
        printf("%s", SHELL_PREFIX);
        fflush(stdout);

        char input[MAX_INPUT];
        unsigned char ch;
        int index = 0;
        int gettingInput = 1;
        while (gettingInput == 1) {
            ch = _getch();
            switch (ch) {
                case ENTER_KEY: {
                    if (index > 0) {
                        input[index] = '\0';
                        history[historyCount++] = strdup(input);
                    }
                    printf("\n");
                    gettingInput = 0;
                    break;
                }
                case TAB_KEY: {
                    strcat(input, "_completed");
                    index += strlen("_completed");
                    printf("\r~ %s", input);  
                    fflush(stdout);
                    break;
                }
                case BACKSPACE: {
                    historyIndex = 0;
                    if(index > 0) {
                        input[--index] = '\0';
                        printf("\r~ %s ", input);
                        printf("\b");
                        fflush(stdout);
                    }
                    break;
                }
                case SPACEBAR: {
                    if(index > 0) {
                        input[index++] = ' ';
                        input[index] = '\0';
                    }
                    printf("\033[C");
                    fflush(stdout);
                    break;
                }
                case 0xE0: {
                    ch = _getch();
                    switch (ch) {
                        case UP_ARROW: {
                            //printf("\n");
                            // printf("---------History---------\n");
                            // for(int i = 0; i < historyCount; i++ ) {
                            //     printf("%s - ", history[i]);
                            // }
                            // printf("\n");
                            // printf("---------History---------\n");
                            if(historyCount > 0) {
                                memset(input, '\0', sizeof(input));
                                strcpy(input, history[historyIndex]);
                                printf("\r\033[K~ %s", history[historyIndex]);
                                historyIndex = (historyIndex + 1) % historyCount;
                                index = strlen(input);
                                fflush(stdout);
                            } 
                            break;
                        }
                        case DOWN_ARROW: {
                            if(historyCount > 0) {
                                memset(input, '\0', sizeof(input));
                                strcpy(input, history[historyIndex]);
                                printf("\r\033[K~ %s", history[historyIndex]);
                                historyIndex = historyIndex > 0 ? historyIndex - 1 : historyCount - 1;
                                index = strlen(input);
                                fflush(stdout);
                            } 
                            break;
                        }   
                        case RIGHT_ARROW: {
                            printf("\033[C");
                            fflush(stdout);
                            break;
                        }
                        case LEFT_ARROW: {
                            printf("\b");
                            fflush(stdout);
                            break;
                        }
                        default: break;
                    }
                    break;
                }
                case 0x03: { // CTRL + C
                    exitShell();
                    break;
                } 
                default: {
                    if(ch > 0x20 && ch < 0x7E) {
                        printf("\033[%d;%dH", getCursorY(), index == 0 ? 3 : index + 3);
                        input[index++] = ch;
                        input[index] = '\0';
                        printf("%c", ch);
                        fflush(stdout);
                    }
                }
            }
        }

        if(index > 0) {
            char *args[MAX_ARGS];
            char *token;
            char *inputCommand;
            int argc = 0;

            token = strtok(input, " \n");
            while(token != NULL && argc < MAX_ARGS - 1) {
                if(argc == 0) {
                    inputCommand = token;
                    strcat(inputCommand, "\0");
                }
                args[argc++] = token;
                token = strtok(NULL, " \n");
            }

            args[argc] = NULL;

            char *lowerCaseCommand = malloc(strlen(inputCommand));
            for(int i = 0; inputCommand[i] != '\0'; i++) {
                lowerCaseCommand[i] = tolower(inputCommand[i]);
            }
            lowerCaseCommand[strlen(inputCommand)] = '\0';   

            for (int i = 0; i < commandCount; i++) {
                if(strcmp(commands[i].name, lowerCaseCommand) == 0) {
                    validCommand = 1;
                    commands[i].func(lowerCaseCommand, args, &argc);
                }
            }

            if (validCommand == 0) {
                printf("'%s' is not a valid command.\n", inputCommand);
            }

            printf("\n");
            free(lowerCaseCommand);
        }
    }

    return 0;
}