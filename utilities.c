#include <stdio.h>  // Input/Output Stream
#include <stdlib.h> // System Commands, Malloc, etc
#include <stdbool.h> // Bool Data Type
#include <sys/stat.h> // Folder Manipulation
#include <dirent.h> // Reading Directories
#include <stdint.h> // Integer Types

char *os() {
#ifdef _WIN32
    return "\\";
#elif __linux__
    return "/";
#else
    printf("This operating system is not supported");
    exit(1);
#endif
}

// Command Manipulation

char *commandOutput(char *command)
{
    int bytes = 200;
    char *str = malloc(bytes);
    FILE *file = popen(command, "r");

    if (file == NULL) {
        printf("Command \"%s\" Failed\n", command);
        exit(1);
    }

    while (fgets(str, bytes, file) != NULL)

    str[strcspn(str, "\n")] = 0;
    pclose(file);
    return str;
}

int supressOutput (char *command)
{
    char *fullcommand = malloc(strlen(command) + 17);
    if (strcmp(os(), "\\") == 0) {
        sprintf(fullcommand, "%s >nul 2>&1", command);
        return system(fullcommand);
    } else if (strcmp(os(), "/") == 0) {
        sprintf(fullcommand, "%s >/dev/null 2>&1", command);
        return system(fullcommand);
    }

    return 1;
}

// File Manipulation

bool fileExists(char *fileName)
{
    FILE *file = fopen(fileName, "r");

    if (file != NULL) {
        fclose(file);
        return true;
    }

    fclose(file);
    return false;
}

void wildcardRename(char *oldName, char *newName)
{
    char *command = malloc(strlen(oldName) + strlen(newName) + 7);

    if (strcmp(os(), "\\") == 0) sprintf(command, "move");
    else if (strcmp(os(), "/") == 0) sprintf(command, "mv");

    sprintf(command, "%s %s %s", command, oldName, newName);
    system(command);
}

char *getExtension(char *fileName)
{
    char* extension = strrchr(fileName, '.');

    if (!extension) return NULL;
    else return extension + 1;
}

char *removeFileExtension(char *fileName)
{
    char *retStr;
    char *lastExt;

    if (fileName == NULL) return NULL;
    if ((retStr = malloc (strlen (fileName) + 1)) == NULL) return NULL;

    strcpy (retStr, fileName);
    lastExt = strrchr (retStr, '.');

    if (lastExt != NULL) *lastExt = '\0';
    return retStr;
}

void copyFile(char *sourceFile, char *destinationFile)
{
    long curPos, endPos;

    FILE *inFile = fopen(sourceFile, "rb");
    FILE *outFile = fopen(destinationFile, "wb");

    if (inFile == NULL) return;
    else if (outFile == NULL) return;

    curPos = ftell(inFile);
    fseek(inFile, 0, 2);
    endPos = ftell(inFile);
    fseek(inFile, curPos, 0);

    size_t bufferLength = endPos;
    uint8_t *buffer = malloc(bufferLength);

    fread(buffer, bufferLength, 1, inFile);
    fwrite(buffer, bufferLength, 1, outFile);

    fclose(inFile);
    fclose(outFile);
}

// Folder Manipulation

void removeFolder(char *folder)
{
    struct stat statpath, statentry;
    struct dirent *entry;

    stat(folder, &statpath);
    S_ISDIR(statpath.st_mode);
    DIR *dir = opendir(folder);
    size_t pathlength = strlen(folder);

    while ((entry = readdir(dir)) != NULL) {
        if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, "..")) continue;

        char *fullpath = calloc(pathlength + 1 + strlen(entry->d_name) + 1, sizeof(char));
        strcpy(fullpath, folder);
        strcat(fullpath, "/");
        strcat(fullpath, entry->d_name);
        stat(fullpath, &statentry);

        if (S_ISDIR(statentry.st_mode) != 0) {
            removeFolder(fullpath);
            free(fullpath);
            continue;
        }

        if (unlink(fullpath) == 0) free(fullpath);
    }

    if (rmdir(folder) == 0) closedir(dir);
}

void copyFolder(char *sourceFolder, char *destinationFolder)
{
    char *command = malloc(strlen(sourceFolder) + strlen(destinationFolder) + 20);

    if (strcmp(os(), "\\") == 0) sprintf(command, "xcopy /E /H /C /I");
    else if (strcmp(os(), "/") == 0) sprintf(command, "cp");

    sprintf(command, "%s %s %s", command, sourceFolder, destinationFolder);
    printf("%s", command);
    system(command);
}