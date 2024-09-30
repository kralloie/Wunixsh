#pragma once

int getCursorY() {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);;
    CONSOLE_SCREEN_BUFFER_INFO csbi;

    if(GetConsoleScreenBufferInfo(hConsole, &csbi)) {
        return csbi.dwCursorPosition.Y + 1;
    } else {
        return 0;
    }
}

int getFilesCount(char* path) {
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
        filesLen[index++] = strlen(fileData.cFileName) + 1;
    } while(FindNextFile(hFind, &fileData) != 0);
    FindClose(hFind);
    return filesLen;
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

void echo(char *inputCommand, char **args, int* argc) {
    int outputLen = 0;

    for (int i = 0; i < *argc; i++) {
        outputLen += strlen(args[i]) + 1;
    }

    char *output = malloc(outputLen * sizeof(char));

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

    if (strlen(output) > 0) {
        output[strlen(output) - 1] = '\0';
    }

    printf("%s\n", output);

    free(output);
    return;
}

void ls (char *inputCommand, char **args, int* argc) {
    WIN32_FIND_DATA findFileData;
    HANDLE hFind;
    char cwd[MAX_PATH];

    if (_getcwd(cwd, sizeof(cwd)) == NULL) {
        return;
    }

    char searchPattern[MAX_PATH];

    snprintf(searchPattern, sizeof(searchPattern), "%s\\*", (*argc > 1) ? args[1] : cwd);

    hFind = FindFirstFile(searchPattern, &findFileData);

    if (hFind == INVALID_HANDLE_VALUE) {
        printf("No files in this directory.\n");
        return;
    }

    do {
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        SetConsoleTextAttribute(hConsole, (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? 10 : 7);
        printf("%s%c\n", findFileData.cFileName, (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? '/' : ' ');
    } while (FindNextFile(hFind, &findFileData) != 0);

    FindClose(hFind);
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

void clear(char* _, char**__, int*___) {
    system("cls");
    return;
}

void exitShell(char*_, char**__, int*___) {
    exit(0);
}
