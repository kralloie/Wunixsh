#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <direct.h>
#include <limits.h>
#include <lmcons.h>
#include "commands.c"

#define SHELL_PREFIX "~ "
#define MAX_INPUT  1024
#define MAX_ARGS 100

typedef void (*CommandFunc)(char*, char**, int*);

typedef struct Command {
    const char* name;
    CommandFunc func;
} Command;

int main() {
    char username[UNLEN + 1];
    DWORD username_len = sizeof(username);
    GetUserName(username, &username_len);

    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

    char input[MAX_INPUT];

    Command commands[] = {
        { "echo", echo },
        { "ls", ls },
        { "cd", cd },
        { "clear", clear }
    };

    int commandc = sizeof(commands) / sizeof(Command);

    while (1) {
        char cwd[MAX_PATH];
        int validCommand = 0;
        if(_getcwd(cwd, sizeof(cwd)) == NULL) {
            printf("Unexpected Error!");
        }
        SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_BLUE);
        printf("\033[1m[%s MinGW64] \033[0m", username);
        SetConsoleTextAttribute(hConsole, FOREGROUND_GREEN | FOREGROUND_BLUE);
        printf("\033[1m%s\033[0m\n", cwd);
        SetConsoleTextAttribute(hConsole, 7);
        printf(SHELL_PREFIX);
        fflush(stdout);

        if (fgets(input, sizeof(input), stdin) == NULL) {
            break;
        }

        if (strcmp(input, "exit\n") == 0) {
            break;
        }

        char *args[MAX_ARGS];
        char *token;
        char *inputCommand;
        int argc = 0;

        token = strtok(input, " \n");
        while(token != NULL && argc < MAX_ARGS - 1) {
            if(argc == 0) {
                inputCommand = token;
            }
            args[argc++] = token;
            token = strtok(NULL, " \n");
        }
        args[argc] = NULL;
        for (int i = 0; i < commandc; i++) {
            if(strcmp(commands[i].name, inputCommand) == 0) {
                validCommand = 1;
                commands[i].func(inputCommand, args, &argc);
            }
        }

        if (validCommand == 0) {
            printf("'%s' is not a valid command.\n", inputCommand);
        }

        printf("\n");
    }

    return 0;
}