#include "main.h"

int main() {
    char *history[MAX_HISTORY];
    int historyCount = 0;
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    GetUserName(username, &username_len);

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
        while (gettingInput) {
            ch = _getch();
            switch (ch) {
                case ENTER_KEY: {
                    if (index > 0) {
                        input[index] = '\0';
                        if (historyCount > 0) {
                            for(int i = 0; i < historyCount; i++) {
                                if (strcmp(input, history[i]) == 0) {
                                    free(history[i]);
                                    for(int j = i; j < historyCount - 1; j++) {
                                        history[j] = strdup(history[j + 1]);
                                    }
                                    historyCount--;
                                    break;
                                }
                            }
                        }
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
                            if(historyCount > 0) {
                                memset(input, '\0', sizeof(input));
                                strcpy(input, history[historyCount - 1 - historyIndex]);
                                printf("\r\033[K~ %s", history[historyCount - 1 - historyIndex]);
                                historyIndex = (historyIndex + 1) % historyCount;
                                index = strlen(input);
                                fflush(stdout);
                            } 
                            break;
                        }
                        case DOWN_ARROW: {
                            if(historyCount > 0) {
                                memset(input, '\0', sizeof(input));
                                strcpy(input, history[historyCount - 1 - historyIndex]);
                                printf("\r\033[K~ %s", history[historyCount - 1 - historyIndex]);
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
                    exitShell(NULL, NULL, NULL);
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