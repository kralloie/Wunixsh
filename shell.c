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
                    if (index > 0 && !getCursorX() - 3 < index) {
                        char path[MAX_PATH];
                        snprintf(path, sizeof(path), "%s\\*", cwd);
                        int fileCount = getFilesCount(path);
                        if (fileCount > 0) {
                            char *tokens[MAX_ARGS];
                            int tokenCount = 0;
                            char *token;
                            char *inputCopy = strdup(input);
                            token = strtok(inputCopy, " \n");
                            while (token != NULL) {
                                tokens[tokenCount++] = strdup(token);
                                token = strtok(NULL, " \n");
                            }
                            char *lastToken = strdup(tokens[tokenCount - 1]);
                            int fileIndex = 0;
                            int *fileLengths = getFilesLen(path, fileCount);
                            char **files = malloc(fileCount * sizeof(char*));
                            for(int i = 0; i < fileCount; i++) {    
                                files[i] = malloc(fileLengths[i] * sizeof(char));
                            }
                            WIN32_FIND_DATA findFileData;
                            HANDLE hFind;
                            hFind = FindFirstFile(path, &findFileData);
                            do {
                                strcpy(files[fileIndex++], findFileData.cFileName);
                            } while(FindNextFile(hFind, &findFileData) != 0);
                            FindClose(hFind);
                            int matchCount = 0;
                            for(int i = 0; i < fileCount; i++) {
                                if(strncmp(files[i], lastToken, strlen(lastToken)) == 0) {
                                    matchCount++;
                                }
                            }
                            
                            if(matchCount == 0) {
                                break;
                            }

                            char **matches = malloc(matchCount * sizeof(char*));
                            int matchIndex = 0;
                            int maxLen = 0;
                            for(int i = 0; i < fileCount; i++) {
                                if(strncmp(files[i], lastToken, strlen(lastToken)) == 0) {
                                    maxLen = (strlen(files[i]) > maxLen) ? strlen(files[i]) : maxLen;
                                    matches[matchIndex] = malloc(strlen(files[i]) * sizeof(char));
                                    strcpy(matches[matchIndex++], files[i]);
                                }
                            }
                            char *match = malloc(maxLen + 1);
                            memset(match, '\0', maxLen + 1);
                            match[0] = lastToken[0];
                            for(int i = 1; i < maxLen; i++) {
                                for(int j = 0; j < matchCount; j++) {
                                    char nextMatchChar = matches[j][i];
                                    int charMatchCount = 0;
                                    for(int k = 0; k < matchCount; k++) {
                                        if (matches[k][i] == nextMatchChar) {
                                            charMatchCount++;
                                        }
                                    }
                                    if (charMatchCount == matchCount) {
                                        match[i] = nextMatchChar;
                                    }
                                }
                            }
                            input[0] = '\0';
                            if(tokenCount > 1) {
                                for (int i = 0; i < tokenCount - 1; i++) {
                                    strcat(input, tokens[i]);
                                    strcat(input, " ");
                                }
                            }

                            strcat(input, match);
                            printf("\r%s%s", SHELL_PREFIX, input);
                            index = strlen(input);  
                            fflush(stdout);
                            
                            free(matches);
                            free(files);
                            free(match);
                        }
                    }
                    break;
                }
                case BACKSPACE: {
                    historyIndex = 0;
                    if(index > 0) {
                        input[--index] = '\0';
                        printf("\r%s%s ", SHELL_PREFIX, input);
                    } 
                    printf("\b");
                    fflush(stdout);
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
                                memset(input, '\0', strlen(input));
                                strcpy(input, history[historyCount - 1 - historyIndex]);
                                printf("\r\033[K%s%s", SHELL_PREFIX, history[historyCount - 1 - historyIndex]);
                                historyIndex = (historyIndex + 1) % historyCount;
                                index = strlen(input);
                                fflush(stdout);
                            } 
                            break;
                        }
                        case DOWN_ARROW: {
                            if(historyCount > 0) {
                                memset(input, '\0', strlen(input));
                                strcpy(input, history[historyCount - 1 - historyIndex]);
                                printf("\r\033[K%s%s", SHELL_PREFIX, history[historyCount - 1 - historyIndex]);
                                historyIndex = historyIndex > 0 ? historyIndex - 1 : historyCount - 1;
                                index = strlen(input);
                                fflush(stdout);
                            } 
                            break;
                        }   
                        case RIGHT_ARROW: {
                            if(getCursorX() - 3 < index) {
                                printf("\033[C");
                                fflush(stdout);
                            }
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
                        int cursorX = getCursorX() - 3;
                        printf("\033[C");
                        printf("\033[s");
                        printf("\033[%d;%dH", getCursorY(), index == 0 ? 3 : index + 3);
                        if(cursorX < index && index > 0) {
                            for(int i = index; i > cursorX; --i) {
                                input[i] = input[i - 1];
                            }
                            input[cursorX] = ch;
                            input[++index] = '\0';
                            printf("\r\033[K%s%s", SHELL_PREFIX, input);
                            printf("\033[u");
                        } else {
                            input[index++] = ch;
                            input[index] = '\0';
                            printf("%c", ch);    
                        }

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
            while (token != NULL && argc < MAX_ARGS - 1) {
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