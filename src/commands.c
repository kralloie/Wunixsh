#include "main.h"

char* history[100];
char username[UNLEN + 1];
DWORD username_len = sizeof(username);
int historyCount = 0;
const Command commands[] = {
    { "echo", echo },
    { "ls", ls },
    { "cd", cd },
    { "clear", clear },
    { "exit", exitShell },
    { "touch", touch },
    { "mkdir", makedir },
    { "rm", rm },
    { "cat", cat },
    { "history", historyCommand },
    { "cp", cp },
    { "mv", mv },
    { "pwd", pwd },
    { "shutdown", shtdwn },
    { "reboot", rstrt },
    { "df", df },
    { "ipconfig", ipconfig },
    { "ping", ping }
};
int commandCount = sizeof(commands) / sizeof(Command);

const char *compressedExtensions[] = { ".tar", ".zip", ".rar", ".arc", ".gz", ".hqx", ".sit" };
const size_t compressedExtensionsSz = sizeof(compressedExtensions);

const char *executableExtensions[] = { ".exe", ".bat", ".com", ".cmd", ".msi" };
const size_t executableExtensionsSz = sizeof(executableExtensions);

void getBranch(char *branch) {
    FILE *fp = popen("git symbolic-ref --short HEAD 2>NUL", "r");
    fgets(branch, 100, fp);
    branch[strcspn(branch, "\n")] = 0;
    pclose(fp);
    return;
}

void prompt(HANDLE hConsole, SYSTEMTIME time, char *cwd, char *prefix, char *username) {
    char branch[100];
    getBranch(branch);
    SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_BLUE);
    printf("\033[1m[%s - %02d:%02d:%02d] - \033[0m", username, time.wHour, time.wMinute, time.wSecond);
    SetConsoleTextAttribute(hConsole, FOREGROUND_GREEN | FOREGROUND_BLUE);
    printf("\033[1m%s\033[0m ", cwd);
    if (branch[0] != '\0') {
        SetConsoleTextAttribute(hConsole, FOREGROUND_GREEN);
        printf("(%s)\n", branch);
    } else {
        printf("\n");
    }
    SetConsoleTextAttribute(hConsole, 7);
    printf("%s", prefix);
    fflush(stdout);
}

char *getQuotatedName(char *str) {
    char *start = strchr(str, '\'');
    if (start == NULL) {
        return NULL;
    }

    char *end = strchr(start + 1, '\'');
    if (end == NULL) {
        return NULL;
    }

    size_t length = end - start - 1;
    char *result = calloc(length + 1, sizeof(char));
    if (result == NULL) {
        return NULL;
    }
    
    strncpy(result, start + 1, length);
    return result;
}

char *strToLower(char* str, int length) {
    char* lowercaseStr = calloc(length + 1, sizeof(char));
    for(int i = 0; i < length; i++) {
        lowercaseStr[i] = tolower(str[i]);
    }
    return lowercaseStr;
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
        free(filesLen);
        return NULL;
    }

    do {
        filesLen[index++] = strlen(fileData.cFileName) + (fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ? 1 : 0) + (strchr(fileData.cFileName, ' ') != NULL ? 2 : 0);
    } while(FindNextFile(hFind, &fileData) != 0);
    FindClose(hFind);
    return filesLen;
}

char **getPathAndFilename(char *path) {
    char *pathCopy = strdup(path);
    char *filePath = strdup(path);
    char *filename = calloc(strlen(path) + 1, sizeof(char));
    char *token = strtok(pathCopy, "/");
    while(token != NULL) {
        strcpy(filename, token);
        token = strtok(NULL, "/");
    }
    filePath[strlen(path) - strlen(filename)] = '\0';
    char **fileNameAndPath = malloc(2 * sizeof(char*));
    fileNameAndPath[0] = strdup(filePath);
    fileNameAndPath[1] = strdup(filename);
    free(filename);
    free(pathCopy);
    return fileNameAndPath;
}

char **getFileNames(char *path) {
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
            char* fileName = calloc(strlen(fileData.cFileName) + 4, sizeof(char));
            strcpy(fileName, fileData.cFileName);
            if (fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                strcat(fileName, "/");
            }
            if (strchr(fileName, ' ') != NULL) {
                char *quotatedFilename = calloc(strlen(fileName) + 3, sizeof(char));
                quotatedFilename[0] = '\'';
                strcat(quotatedFilename, fileName);
                strcat(quotatedFilename, "'");
                free(fileName);
                fileName = quotatedFilename;
            }
            strcpy(fileNames[fileIndex++], fileName);
        } while(FindNextFile(hFind, &fileData) != 0);
        FindClose(hFind);
        return fileNames;
    }
    return NULL;
}

int checkExtension(char *fileName, const char *extensions[], const size_t *extSz) {
    int extIndex = *extSz / sizeof(extensions[0]);
    if(strlen(fileName) > strlen(extensions[0])) {
        for(int i = 0; i < extIndex; i++) {
            if(strstr(fileName + (strlen(fileName) - strlen(extensions[i])) - 1, extensions[i]) != NULL) {
                return 1;
            }
        }
    }
    return 0;
}

int printFileName(WIN32_FIND_DATA *fileData) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD attributes = fileData->dwFileAttributes;
    char *fileName = strdup(fileData->cFileName);
    int offset = 0;
    if (attributes == INVALID_FILE_ATTRIBUTES) {
        return 0;
    }

    if (strchr(fileName, ' ') != NULL) {
        char *quotatedFilename = calloc(strlen(fileName) + 3, sizeof(char));
        quotatedFilename[0] = '\'';
        strcat(quotatedFilename, fileName);
        strcat(quotatedFilename, "'");
        free(fileName);
        fileName = quotatedFilename;
        offset = 2;
    }

    if (attributes & FILE_ATTRIBUTE_REPARSE_POINT) {
        SetConsoleTextAttribute(hConsole, FOREGROUND_GREEN | FOREGROUND_BLUE);
        printf("\033[1m%s\033[0m", fileName);
        SetConsoleTextAttribute(hConsole, 7);
        printf("@");
        return offset + 1;
    }

    if (attributes & FILE_ATTRIBUTE_DIRECTORY) {
        SetConsoleTextAttribute(hConsole, FOREGROUND_BLUE);
        printf("\033[1m%s\033[0m", fileName);
        SetConsoleTextAttribute(hConsole, 7);
        printf("/");
        return offset + 1;
    }

    if (checkExtension(fileName, executableExtensions, &executableExtensionsSz)) {
        SetConsoleTextAttribute(hConsole, FOREGROUND_GREEN);
        printf("\033[1m%s\033[0m", fileName);
        return offset;
    }
 
    if (checkExtension(fileName, compressedExtensions, &compressedExtensionsSz)) {
        SetConsoleTextAttribute(hConsole, FOREGROUND_RED);
        printf("%s", fileName);
        return offset;
    }

    SetConsoleTextAttribute(hConsole, 7);
    printf("%s", fileName);
    return offset;
}

int hasAlphanumeric(char *arg) {
    int alphanumericCharCount = 0;
    for (int i = 0; i < strlen(arg); i++) {
        alphanumericCharCount += isalnum(arg[i]);
    }
    return alphanumericCharCount > 0;
}

void enableShutdownPrivilege() {
    HANDLE hToken;
    TOKEN_PRIVILEGES tkp;

    if(!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) {
        printf("Failed to get process token.");
        return;
    }

    LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &tkp.Privileges[0].Luid);
    tkp.PrivilegeCount = 1;
    tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
    AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES)NULL, 0);
    CloseHandle(hToken);
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
        int offset = printFileName(&fileData);

        if(terminalLen < fileTotalLen) {
            columnCounter++;
            if(columnCounter < columns - 1) {
                for(int i = 0; i < fileMaxLen - strlen(fileData.cFileName) + space - offset; i++) {
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

void cat(char *inputCommand, char **args, int *argc) {
    char fileBuffer[256];
    if (*argc > 1) {
        if (hasAlphanumeric(args[1])) {
            FILE *file = fopen(args[1], "r");
            if (file == NULL) {
                printf("File doesn't exists.");
                return;
            }
            while (fgets(fileBuffer, sizeof(fileBuffer), file) != NULL){
                printf("%s", fileBuffer);
            }
            fclose(file);
        } else {
            printf("Invalid file name.\n");
        }
        return;
    }
}

void touch(char *inputCommand, char **args, int *argc) {
    if (*argc > 1) {
        if (hasAlphanumeric(args[1])) {
            FILE *file = fopen(args[1], "w");
            if (file == NULL) {
                printf("Error creating file.\n");
                return;
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

void makedir(char *inputCommand, char **args, int *argc) {
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

void rm(char *inputCommand, char **args, int *argc) {
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

void historyCommand(char *inputCommand, char **args, int *argc) {
    if(historyCount == 1) {
        printf("History is empty.\n");
        return;
    }
    for(int i = 0; i < historyCount - 1; i++) {
        printf(" %d %s\n", i, history[i]);
    }
    return;
};

void cp(char *inputCommand, char **args, int *argc) {
    if(*argc > 2) {
        if(hasAlphanumeric(args[1])) {
            char *destiny = calloc(strlen(args[2]) + strlen(args[1]), sizeof(char));
            strcpy(destiny, args[2]);
            if (destiny[strlen(destiny) - 1] == '/' || destiny[strlen(destiny) - 1] == '.') {
                char **filename = getPathAndFilename(args[1]);
                strcat(destiny, "/");
                strcat(destiny, filename[1]);
                free(filename);
            }
            BOOL result = CopyFile(args[1], destiny, FALSE);
            free(destiny);
            if (result) {
                return;
            } else {
                printf("Failed to move file.\n");
            }
        } else {
            printf("Invalid file name.\n");
        }
    } else {
        printf("Insufficient arguments.\n");
    }
    return;
}

void mv(char *inputCommand, char **args, int *argc) {
    if(*argc > 2) {
        if(hasAlphanumeric(args[1])) {
            char *destiny = calloc(strlen(args[2]) + strlen(args[1]), sizeof(char));
            strcpy(destiny, args[2]);
            if (destiny[strlen(destiny) - 1] == '/' || destiny[strlen(destiny) - 1] == '.') {
                char **filename = getPathAndFilename(args[1]);
                strcat(destiny, "/");
                strcat(destiny, filename[1]);
                free(filename);
            }
            BOOL result = MoveFile(args[1], destiny);
            if (result) {
                return;
            } else {
                printf("Failed to move file.\n");
            }
        } else {
            printf("Invalid file name.\n");
        }
    } else {
        printf("Insufficient arguments.\n");
    }
    return;
}

void cd(char *inputCommand, char **args, int *argc) {
    if(*argc < 2) {
        return;
    }
    if(_chdir(args[1]) != 0) {
        printf("Invalid directory.\n");
    }
    return;
}

void pwd(char *_, char **__, int *___) {
    char cwd[MAX_PATH];
    if(_getcwd(cwd, sizeof(cwd)) == NULL) {
        printf("Unexpected Error.");
        return;
    }
    printf("Curent Working Directory: %s\n", cwd);
    return;
}

void shtdwn(char *_, char **__, int *___) {
    enableShutdownPrivilege();
    ExitWindowsEx(EWX_SHUTDOWN | EWX_FORCE, SHTDN_REASON_MAJOR_SOFTWARE);
}

void rstrt(char *_, char **__, int *___) {
    enableShutdownPrivilege();
    ExitWindowsEx(EWX_REBOOT | EWX_FORCE, SHTDN_REASON_MAJOR_SOFTWARE);
}

void clear(char *_, char **__, int *___) {
    system("cls");
    return;
}

void exitShell(char *_, char **__, int *___) {
    exit(0);
}

void df(char *_, char **__, int *___) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD driveMask = GetLogicalDrives();
    char drives[256];
    DWORD driveCount = GetLogicalDriveStrings(sizeof(drives), drives);
    if (driveCount == 0) {
        printf("Error getting logical drives.\n");
        return;
    }
    SetConsoleTextAttribute(hConsole, FOREGROUND_BLUE);
    printf("Disk    ");
    printf("Total Space  ");
    printf("Free Space   ");
    printf("Use %%        \n");
    SetConsoleTextAttribute(hConsole, 7);
    int outputPadding = 13;
    for(char *drive = drives; *drive != '\0'; drive += 4) {
        if (GetDriveType(drive) == DRIVE_FIXED) {
            ULARGE_INTEGER freeBytesAvailable, totalBytes, totalFreeBytes;
            if (GetDiskFreeSpaceEx(drive, &freeBytesAvailable, &totalBytes, &totalFreeBytes)) {
                unsigned long long totalBytesGB = totalBytes.QuadPart / (1024 * 1024 * 1024);
                unsigned long long totalFreeBytesGB = totalFreeBytes.QuadPart / (1024 * 1024 * 1024);
                char totalBytesStr[20];
                char totalFreeBytesStr[20];
                snprintf(totalBytesStr, sizeof(totalBytesStr), "%lluGB", totalBytesGB);
                snprintf(totalFreeBytesStr, sizeof(totalFreeBytesStr), "%lluGB", totalFreeBytesGB);
                printf("\033[1m%s\033[0m%*s" , drive, 5, "");
                printf("\033[1m%s\033[0m%*s", totalBytesStr, outputPadding - strlen(totalBytesStr), "");
                printf("\033[1m%s\033[0m%*s", totalFreeBytesStr, outputPadding - strlen(totalFreeBytesStr), "");
                printf("\033[1m%d%%\033[0m\n", (int)ceil((((double)(totalBytesGB - totalFreeBytesGB)) / totalBytesGB) * 100));
            }   
        }
    }
    return;
}

void ipconfig(char *_, char **__, int *___) {
    system("ipconfig");
    return;
}

void ping(char *inputCommand, char **args, int *argc) {
    if (*argc < 2) {
        printf("Insufficient arguments.\n");
        return;
    }
    char* pingCommand = calloc(strlen(args[0]) + strlen(args[1]) + 2, sizeof(char));
    snprintf(pingCommand, strlen(args[0]) + strlen(args[1]) + 2, "%s %s", inputCommand, args[1]);
    system(pingCommand);
    return;
}

// --------- Commands ---------

void executeCommand(char *input) {
    int validCommand = 0;
    char *inputCopy = strdup(input);
    char *args[MAX_ARGS];
    char *token;
    char *inputCommand;
    int argc = 0;
    char *quotatedArgs[MAX_ARGS];
    int quotatedArgsCounter = 0;
    int quotatedArgsIndex = 0;

    char *pipe = strstr(input, "||");

    for(int i = 0; i < strlen(input); i++) {
        if (isalnum(input[i])) {
            break;
        }
        if (input[i] == '|') {
            return;
        }
    }

    if (pipe != NULL) {
        input[pipe - input] = '\0';
    }

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

    char *nextCommand = strstr(inputCopy, "||");
    if (nextCommand != NULL) {
        if (strlen(nextCommand) <= 2) {
            return;
        }
        executeCommand(nextCommand + 2);
    }
}
