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

const char *compressedExtensions[] = { ".tar", ".zip", ".rar", ".arc", ".gz", ".hqx", ".sit" };
const size_t compressedExtensionsSz = sizeof(compressedExtensions);

const char *executableExtensions[] = { ".exe", ".bat", ".com", ".cmd", ".msi" };
const size_t executableExtensionsSz = sizeof(executableExtensions);

void prompt(HANDLE hConsole, SYSTEMTIME time, char *cwd, char *prefix, char *username) {
    SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_BLUE);
    printf("\033[1m[%s - %02d:%02d:%02d] - \033[0m", username, time.wHour, time.wMinute, time.wSecond);
    SetConsoleTextAttribute(hConsole, FOREGROUND_GREEN | FOREGROUND_BLUE);
    printf("\033[1m%s\033[0m\n", cwd);
    SetConsoleTextAttribute(hConsole, 7);
    printf("%s", prefix);
    fflush(stdout);
}

int getCursorY() {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if(GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) {
        return csbi.dwCursorPosition.Y + 1;
    } else {
        return 0;
    }
}

int getTerminalLength() {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) {
        return csbi.srWindow.Right - csbi.srWindow.Left + 1;
    }
}

int getCursorX() {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if(GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) {
        return csbi.dwCursorPosition.X + 1;
    } else {
        return 0;
    }
}

int getFilesCount(char *path) {
    int fileCount  = 0;
    WIN32_FIND_DATA fileData;
    HANDLE hFind;
    hFind = FindFirstFile(path, &fileData);
    if (hFind == INVALID_HANDLE_VALUE) {
        return 0;
    }
    do {
        fileCount++;
    } while(FindNextFile(hFind, &fileData) != 0);
    FindClose(hFind);
    return fileCount;
}

int *getFilesLen(char *path, const int fileCount) {
    int *filesLen = malloc(fileCount * sizeof(int));
    int index = 0;
    WIN32_FIND_DATA fileData;
    HANDLE hFind;
    hFind = FindFirstFile(path, &fileData);
    if (hFind == INVALID_HANDLE_VALUE) {
        return 0;
    }

    do {
        filesLen[index++] = strlen(fileData.cFileName) + (fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ? 1 : 0);
    } while(FindNextFile(hFind, &fileData) != 0);
    FindClose(hFind);
    return filesLen;
}

char **getFileNames(char* path) {
    int fileCount = getFilesCount(path);
    if (fileCount > 0) {
        int *fileLengths = getFilesLen(path, fileCount);
        char **fileNames = malloc(fileCount * sizeof(char*));
        for (int i = 0; i < fileCount; i++) {
            fileNames[i] = calloc(fileLengths[i] + 1, sizeof(char));
        }
        int fileIndex = 0;
        WIN32_FIND_DATA fileData;
        HANDLE hFind;
        hFind = FindFirstFile(path, &fileData);
        do {
            char* fileName = fileData.cFileName;
            if (fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                strcat(fileName, "/");
            }
            strcpy(fileNames[fileIndex++], fileName);
        } while(FindNextFile(hFind, &fileData) != 0);
        FindClose(hFind);
        return fileNames;
    }
    return NULL;
}

int checkExtension(char* fileName, const char *extensions[], const size_t *extSz) {
    int extIndex = *extSz / sizeof(extensions[0]);
    if(strlen(fileName) > strlen(extensions[0])) {
        for(int i = 0; i < extIndex; i++) {
            if(strcmp(fileName + (strlen(fileName) - strlen(extensions[i])), extensions[i]) == 0) {
                return 1;
            }
        }
    }
    return 0;
}

int printFileName(WIN32_FIND_DATA *fileData) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD attributes = fileData->dwFileAttributes;
    char* fileName = strdup(fileData->cFileName);
    if (attributes == INVALID_FILE_ATTRIBUTES) {
        return 0;
    }

    if (attributes & FILE_ATTRIBUTE_REPARSE_POINT) {
        SetConsoleTextAttribute(hConsole, FOREGROUND_GREEN | FOREGROUND_BLUE);
        printf("\033[1m%s@\033[0m", fileName);
        return 1;
    }

    if (attributes & FILE_ATTRIBUTE_DIRECTORY) {
        SetConsoleTextAttribute(hConsole, FOREGROUND_BLUE);
        printf("\033[1m%s/\033[0m", fileName);
        return 1;
    }

    if (checkExtension(fileName, executableExtensions, &executableExtensionsSz)) {
        SetConsoleTextAttribute(hConsole, FOREGROUND_GREEN);
        printf("\033[1m%s\033[0m", fileName);
        return 0;
    }
 
    if (checkExtension(fileName, compressedExtensions, &compressedExtensionsSz)) {
        SetConsoleTextAttribute(hConsole, FOREGROUND_RED);
        printf("%s", fileName);
        return 0;
    }

    SetConsoleTextAttribute(hConsole, 7);
    printf("%s", fileName);
    return 0;
}

void printHistory(char **history, int *historyCount) {
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

int hasAlphanumeric(char *arg) {
    int alphanumericCharCount = 0;
    for (int i = 0; i < strlen(arg); i++) {
        alphanumericCharCount += isalnum(arg[i]);
    }
    return alphanumericCharCount > 0;
}

// --------- Commands ---------

void echo(char *inputCommand, char **args, int *argc) {
    int outputLen = 0;

    for (int i = 0; i < *argc; i++) {
        outputLen += strlen(args[i]) + 1;
    }

    char *output = malloc(outputLen * sizeof(char) + 1);

    if (output == NULL) {
        return;
    }

    output[0] = '\0';
    for (int i = 0; i < *argc; i++) {
        if (strcmp(args[i], inputCommand) != 0) {
            strcat(output, args[i]);
            strcat(output, " ");
        }
    }

    printf("%s\n", output);

    free(output);
    return;
}

void ls (char *inputCommand, char **args, int *argc) {
    WIN32_FIND_DATA fileData;
    HANDLE hFind;
    char cwd[MAX_PATH];

    if (_getcwd(cwd, sizeof(cwd)) == NULL) {
        return;
    }

    char searchPattern[MAX_PATH];

    snprintf(searchPattern, sizeof(searchPattern), "%s/*", (*argc > 1) ? args[1] : cwd);

    hFind = FindFirstFile(searchPattern, &fileData);

    if (hFind == INVALID_HANDLE_VALUE) {
        printf("No files in this directory.\n");
        return;
    }

    int fileCount = getFilesCount(searchPattern);
    int fileMaxLen = 0;
    int fileTotalLen = 0;
    char **fileNames = getFileNames(searchPattern);

    for(int i = 0; i < fileCount; i++) {
        fileMaxLen = strlen(fileNames[i]) > fileMaxLen ? strlen(fileNames[i]) + 1 : fileMaxLen;
        fileTotalLen += strlen(fileNames[i]);
    }

    int terminalLen = getTerminalLength();
    int columns = terminalLen / fileMaxLen;
    int columnCounter = 0;
    const int space = 3;
    do {
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        int isDir = fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;
        SetConsoleTextAttribute(hConsole, (isDir) ? 10 : 7);
        int dirOffset = printFileName(&fileData);

        if(terminalLen < fileTotalLen) {
            columnCounter++;
            if(columnCounter < columns - 1) {
                for(int i = 0; i < fileMaxLen - strlen(fileData.cFileName) + space - dirOffset; i++) {
                    printf(" ");
                }
            }
            if (columnCounter >= columns - 1) {
                columnCounter = 0;
                printf("\n");
            } 
        } else { 
            printf("  ");
        }

    } while (FindNextFile(hFind, &fileData) != 0);
    
    FindClose(hFind);
    printf("\n");
    return;
}

void touch(char *inputCommand, char **args, int* argc) {
    if (*argc > 1) {
        if (hasAlphanumeric(args[1])) {
            FILE *file = fopen(args[1], "w");
            if (file == NULL) {
                printf("Error creating file.\n");
            }
            fclose(file);
            printf("File '%s' created successfully\n", args[1]);
        } else {
            printf("Invalid file name.\n");
        }
        return;
    }
    printf("Insufficient arguments.\n");
    return;
}

void makedir(char *inputCommand, char **args, int* argc) {
    if (*argc > 1) {
        if (hasAlphanumeric(args[1])) {
            if (CreateDirectory(args[1], NULL)) {
                printf("Directory '%s' created successfully\n", args[1]);
            } else {
                if(GetLastError() == ERROR_ALREADY_EXISTS) {
                    printf("Directory already exists.\n");
                }
            }
            return;
        } else {
            printf("Invalid folder name.\n");
        }
        return;
    }
    printf("Insufficient arguments.\n");
    return;
}

void rm(char *inputCommand, char **args, int* argc) {
    if (*argc > 1) {
        DWORD attributes = GetFileAttributes(args[1]);
        if (attributes == INVALID_FILE_ATTRIBUTES) {
            printf("Invalid name specified.\n");
            return;
        }
        printf("\nAre you sure? (y/n): ");
        char confirmation = _getch();
        printf("\rAre you sure? (y/n): %c", confirmation);
        if(tolower(confirmation) == 'y') {
            if (attributes & FILE_ATTRIBUTE_DIRECTORY) {
                RemoveDirectory(args[1]);
            } else {
                DeleteFile(args[1]);
            }
        }
        return;
    }
    printf("Please, specify a file/directory name to remove.\n");
    return;
}

void cd(char *inputCommand, char **args, int* argc) {
    if(*argc < 2) {
        return;
    }
    
    if(_chdir(args[1]) != 0) {
        printf("Invalid directory.\n");
    }

    return;
}

void clear(char *_, char**__, int *___) {
    system("cls");
    return;
}

void exitShell(char *_, char **__, int *___) {
    exit(0);
}

// --------- Commands ---------
