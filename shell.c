#include "main.h"

int main() {
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

        prompt(hConsole, time, cwd, SHELL_PREFIX, username);

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
                        int isPath = 0;
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
                        if(lastToken == NULL) {
                            lastToken = strdup(input);
                        }
                        
                        if(lastToken[strlen(lastToken) - 1] == '/') {
                            break;
                        }

                        char path[MAX_PATH];
                        snprintf(path, sizeof(path), "%s/", cwd);
                        char *pathToken = strdup(lastToken);
                        if(strchr(lastToken, '/') != NULL) {
                            char **pathAndFilename = getPathAndFilename(lastToken);
                            if (strchr(lastToken, ':') != NULL || strchr(lastToken, '\'')) {
                                snprintf(path, sizeof(path), "%s*", pathAndFilename[0]);
                            } else {
                                snprintf(path, sizeof(path), "%s/%s*", cwd, lastToken);
                            }
                            isPath = 1;
                            strcpy(pathToken, pathAndFilename[0]);
                            strcpy(lastToken, pathAndFilename[1]);
                            free(pathAndFilename);

                        } else {
                            strcat(path, "*");
                        }

                        int fileCount = getFilesCount(path);
                        if (fileCount > 0) {
                            int fileIndex = 0;
                            char *lowercaseToken = strToLower(lastToken, strlen(lastToken));
                            char **files = getFileNames(path);
                            int matchCount = 0;
                            for(int i = 0; i < fileCount; i++) {
                                char* lowercaseFile = strToLower(files[i], strlen(files[i]));
                                if(strncmp(lowercaseFile, lowercaseToken, strlen(lowercaseToken)) == 0) {
                                    matchCount++;
                                }
                                free(lowercaseFile);
                            }
                            if(matchCount == 0) {
                                break;
                            }

                            char **matches = malloc(matchCount * sizeof(char*));
                            int matchIndex = 0;
                            int maxLen = 0;
                            for(int i = 0; i < fileCount; i++) {
                                char *lowercaseFile = strToLower(files[i], strlen(files[i]));
                                if(strncmp(lowercaseFile, lowercaseToken, strlen(lowercaseToken)) == 0) {
                                    int fileNameLength = strlen(files[i]);
                                    maxLen = (fileNameLength > maxLen) ? fileNameLength : maxLen;
                                    matches[matchIndex] = malloc(fileNameLength * sizeof(char) + 1);
                                    strcpy(matches[matchIndex++], files[i]);
                                }
                                free(lowercaseFile);
                            }
                            free(lowercaseToken);

                            char *match = calloc(maxLen + 1, sizeof(char));
                            for(int i = 0; i < maxLen; i++) {
                                for(int j = 0; j < matchCount; j++) {
                                    char nextMatchChar = matches[j][i];
                                    int charMatchCount = 0;
                                    for(int k = 0; k < matchCount; k++) {
                                        if(strlen(matches[k]) >= i) {
                                            if (tolower(matches[k][i]) == tolower(nextMatchChar)) {
                                                charMatchCount++;
                                            }
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
                            if (isPath) {
                                strcat(input, pathToken); 
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
                        printf("\033[s");
                        int cursorX = getCursorX() - 3;
                        if(cursorX < index) {
                            for(int i = cursorX - 1; i < index - 1; i++) {
                                input[i] = input[i + 1];
                            }
                        }
                        input[--index] = '\0';
                        printf("\r\033[K%s%s", SHELL_PREFIX, input);
                        printf("\033[u");
                    } 
                    if(getCursorX() > 3) {
                        printf("\b");
                    } 
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
                            if(getCursorX() > 3) {
                                printf("\b");
                                fflush(stdout);
                            } 
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
            char *quotatedArgs[MAX_ARGS];
            int quotatedArgsCounter = 0;
            int quotatedArgsIndex = 0;

            while (strchr(input, '\'') != NULL) {
                quotatedArgs[quotatedArgsCounter++] = getQuotatedName(input);
                char *start = strchr(input, '\'');
                char *end = strchr(start + 1, '\'');
                if (end == NULL) {
                    break;
                }
                for (char *p = start; p <= end; p++) {
                    *p = '?';
                }
            }
            token = strtok(input, " \n");
            while (token != NULL && argc < MAX_ARGS - 1) {
                if(argc == 0) {
                    inputCommand = strdup(token);
                }
                args[argc++] = token;
                token = strtok(NULL, " \n");
            }
            args[argc] = NULL;

            for(int i = 0; i < argc; i++) {
                if (strchr(args[i], '?') != NULL) {
                    if (strchr(args[i], '/') != NULL) {
                        char **pathAndFilename = getPathAndFilename(args[i]);
                        if(strlen(pathAndFilename[1]) == strlen(quotatedArgs[quotatedArgsIndex]) + 2) { // + 2 = 2 quotes.
                            free(args[i]);
                            args[i] = calloc(strlen(pathAndFilename[1]) + strlen(pathAndFilename[0]) + 1, sizeof(char));
                            strcpy(args[i], pathAndFilename[0]);
                            strcat(args[i], quotatedArgs[quotatedArgsIndex++]);
                        }
                        free(pathAndFilename[0]);
                        free(pathAndFilename[1]);
                        free(pathAndFilename);
                    } else {
                        if(strlen(args[i]) == strlen(quotatedArgs[quotatedArgsIndex]) + 2) {
                            args[i] = quotatedArgs[quotatedArgsIndex++];
                        } 
                    }
                }
            }

            char *lowerCaseCommand = calloc(strlen(inputCommand) + 1, sizeof(char));
            for(int i = 0; inputCommand[i] != '\0'; i++) {
                lowerCaseCommand[i] = tolower(inputCommand[i]);
            }

            for (int i = 0; i < commandCount; i++) {
                if(strcmp(commands[i].name, lowerCaseCommand) == 0) {
                    validCommand = 1;
                    commands[i].func(lowerCaseCommand, args, &argc);
                }
            }

            if (validCommand == 0) {
                if (strchr(inputCommand, '?') != NULL && quotatedArgsCounter > 0) {
                    free(inputCommand);
                    inputCommand = quotatedArgs[0];
                }
                printf("'%s' is not a valid command.\n", inputCommand);
            }

            printf("\n");
            free(lowerCaseCommand);
        }
    }

    return 0;
}